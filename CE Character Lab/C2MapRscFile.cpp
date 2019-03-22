//
//  C2MapRscFile.cpp
//  CE Character Lab
//
//  Created by Tyler Minard on 8/14/15.
//  Copyright (c) 2015 Tyler Minard. All rights reserved.
//

#include "C2MapRscFile.h"
#include <iostream>
#include <fstream>

#include "CETexture.h"
#include "C2WorldModel.h"
#include "C2Sky.h"

template <typename T, typename... Args>
auto make_unique(Args&&... args) -> std::unique_ptr<T>
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
void Console_PrintLogString(std::string log_msg);

C2MapRscFile::C2MapRscFile(const std::string& file_name)
{
    this->load(file_name);
}

C2MapRscFile::~C2MapRscFile()
{
    
}

CETexture* C2MapRscFile::getTexture(int i)
{
    return this->m_textures.at(i).get();
}

C2WorldModel* C2MapRscFile::getWorldModel(int i)
{
    if (i < 0 || i >= this->m_models.size()) {
        return nullptr;
    }
    
    return this->m_models.at(i).get();
}

int C2MapRscFile::getTextureAtlasWidth()
{
    return this->m_texture_atlas_width;
}

C2Sky* C2MapRscFile::getDaySky()
{
    return this->m_day_sky.get();
}

void C2MapRscFile::load(const std::string &file_name)
{
    std::ifstream infile;
    infile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    try {
        infile.open(file_name.c_str(), std::ios::binary | std::ios::in);

        int texture_count, model_count;
        int SOURCE_SQUARE_SIZE = 128; // each texture in C2 is 128x128
        int TEXTURE_SQUARE_SIZE = 130; // we add 2 px to each side for clamping, for 130x130
        
        infile.read(reinterpret_cast<char *>(&texture_count), 4);
        infile.read(reinterpret_cast<char *>(&model_count), 4);
        
        infile.read(reinterpret_cast<char *>(this->m_fade_rgb), 4*3*3);
        infile.read(reinterpret_cast<char *>(this->m_trans_rgb), 4*3*3);
        
        std::vector<uint16_t> raw_texture_data; //rgba5551
        raw_texture_data.resize(SOURCE_SQUARE_SIZE*SOURCE_SQUARE_SIZE*texture_count);
        infile.read(reinterpret_cast<char *>(raw_texture_data.data()), SOURCE_SQUARE_SIZE*SOURCE_SQUARE_SIZE*sizeof(uint16_t)*texture_count);

        // Combine all the textures into a single texture for ease of opengl use.
        // Add a buffer around images to mimic clamping with GL_LINEAR; see: https://stackoverflow.com/questions/19611745/opengl-black-lines-in-between-tiles
        std::vector<uint16_t> combined_texture_data; // rgba5551

        int squared_texture_rows = static_cast<int>(sqrt(static_cast<float>(texture_count)) + .99f); // number of rows and columns needed to fit the data
        
        uint16_t pad_color;
        std::vector<uint16_t> tx_filler(TEXTURE_SQUARE_SIZE*squared_texture_rows);

        for (int line = 0; line < (squared_texture_rows*SOURCE_SQUARE_SIZE); line++) { // Total vertical rows in image file (128 * number of images; current = pointer to vertical pixel location in file that represents the location of a new row of horizontal blocks)
            
            if (line % SOURCE_SQUARE_SIZE == 0) {
                pad_color = _PadTypeColor::Red;
                tx_filler.clear();
                tx_filler.assign(TEXTURE_SQUARE_SIZE*squared_texture_rows, pad_color);

                for (int x=0; x < squared_texture_rows; x++) {
                    tx_filler.at(x*TEXTURE_SQUARE_SIZE) = _PadTypeColor::Blue;
                    tx_filler.at((x*TEXTURE_SQUARE_SIZE)+(TEXTURE_SQUARE_SIZE-1)) = _PadTypeColor::Green;
                }
                combined_texture_data.insert(combined_texture_data.end(), tx_filler.begin(), tx_filler.end());
            }
            
            for (int col = 0; col < squared_texture_rows; col++) { // Blocks (number of images; current = current block's image)
                int lin_to_row = static_cast<int>(line/SOURCE_SQUARE_SIZE);
                int tx_c = ((lin_to_row)*squared_texture_rows)+col;
                int tex_start = line % SOURCE_SQUARE_SIZE;
                
                pad_color = _PadTypeColor::Blue;
                combined_texture_data.insert(combined_texture_data.end(), pad_color);
                
                if (tx_c >= texture_count) {
                    std::vector<uint16_t> tx_filler(SOURCE_SQUARE_SIZE);
                    combined_texture_data.insert(combined_texture_data.end(), tx_filler.begin(), tx_filler.end());
                } else {
                    std::vector<uint16_t>::const_iterator first = raw_texture_data.begin() + (tx_c*SOURCE_SQUARE_SIZE*SOURCE_SQUARE_SIZE) + (tex_start*SOURCE_SQUARE_SIZE); // Get start pos of a single line of pixels
                    std::vector<uint16_t>::const_iterator last = raw_texture_data.begin() + (tx_c*SOURCE_SQUARE_SIZE*SOURCE_SQUARE_SIZE) + (tex_start*SOURCE_SQUARE_SIZE) + SOURCE_SQUARE_SIZE; // Get end pos, which is the start + 128 pixels
                    combined_texture_data.insert(combined_texture_data.end(), first, last);
                }

                pad_color = _PadTypeColor::Green;
                combined_texture_data.insert(combined_texture_data.end(), pad_color);
            }
            
            if ((line+1) % SOURCE_SQUARE_SIZE == 0) {
                pad_color = _PadTypeColor::Yellow;
                tx_filler.clear();
                tx_filler.assign(TEXTURE_SQUARE_SIZE*squared_texture_rows, pad_color);
                
                for (int x=0; x < squared_texture_rows; x++) {
                    tx_filler.at(x*TEXTURE_SQUARE_SIZE) = _PadTypeColor::Blue;
                    tx_filler.at((x*TEXTURE_SQUARE_SIZE)+(TEXTURE_SQUARE_SIZE-1)) = _PadTypeColor::Green;
                }
                
                combined_texture_data.insert(combined_texture_data.end(), tx_filler.begin(), tx_filler.end());
            }
        }
        
        int missing_bits = (squared_texture_rows*TEXTURE_SQUARE_SIZE*squared_texture_rows*TEXTURE_SQUARE_SIZE) - static_cast<int>(combined_texture_data.size());
        
        if (missing_bits > 0) {
            std::vector<uint16_t> tx_filler(missing_bits);
            combined_texture_data.insert(combined_texture_data.end(), tx_filler.begin(), tx_filler.end());
        }
        
        std::vector<uint16_t> final_texture_data; // Now replace colors with correct entries
        for (int i=0; i < 2; i++) {
            for (int row = 0; row < (TEXTURE_SQUARE_SIZE*squared_texture_rows); row++) {
                for (int col = 0; col < (TEXTURE_SQUARE_SIZE*squared_texture_rows); col++) {
                    int id_x = col;
                    int id_y = row*TEXTURE_SQUARE_SIZE*squared_texture_rows;
                    uint16_t color;
                    
                    if (i == 1) {
                        color = final_texture_data.at(id_y+id_x);
                    } else {
                        color = combined_texture_data.at(id_y+id_x);
                    }
                    
                    switch (color) {
                        case _AtlasPadType::Left:
                            color = final_texture_data.at(id_y+(id_x-1));
                            break;
                        case _AtlasPadType::Right:
                            if (i == 1) {
                                color = final_texture_data.at(id_y+(id_x+1));
                            } else {
                                color = combined_texture_data.at(id_y+(id_x+1));
                            }
                            break;
                        case _AtlasPadType::Above:
                            color = final_texture_data.at(((row-1)*TEXTURE_SQUARE_SIZE*squared_texture_rows)+id_x);
                            break;
                        case _AtlasPadType::Below:
                            if (i == 1) {
                                color = final_texture_data.at(((row+1)*TEXTURE_SQUARE_SIZE*squared_texture_rows)+id_x);
                            } else {
                                color = combined_texture_data.at(((row+1)*TEXTURE_SQUARE_SIZE*squared_texture_rows)+id_x);
                            }
                            break;
                        default:
                            break;
                    }
                    if (i == 1) {
                        final_texture_data.at(id_y+id_x) = color;
                    } else {
                        final_texture_data.insert(final_texture_data.end(), color);
                    }
                }
            }
        }
        
        std::unique_ptr<CETexture> cTexture = std::unique_ptr<CETexture>(new CETexture(final_texture_data, squared_texture_rows*TEXTURE_SQUARE_SIZE*squared_texture_rows*TEXTURE_SQUARE_SIZE, squared_texture_rows*TEXTURE_SQUARE_SIZE, squared_texture_rows*TEXTURE_SQUARE_SIZE));
        this->m_texture_atlas_width = squared_texture_rows;
        this->m_texture_count = texture_count;
        this->m_textures.push_back(std::move(cTexture));
        
        
        // Load 3d models
        for (int m=0; m < model_count; m++) {
            std::unique_ptr<C2WorldModel> cModel = std::unique_ptr<C2WorldModel>(new C2WorldModel(infile));
            this->m_models.push_back(std::move(cModel));
        }
        
        // Load sky bitmap and map overlay (dawn, day, night)
        this->m_dawn_sky = std::unique_ptr<C2Sky>(new C2Sky(infile));
        this->m_day_sky = std::unique_ptr<C2Sky>(new C2Sky(infile));
        this->m_night_sky = std::unique_ptr<C2Sky>(new C2Sky(infile));
        
        this->m_shadow_map.resize(128*128);
        infile.read(reinterpret_cast<char *>(this->m_shadow_map.data()), 128*128);
        
        // Load fog data
        // Load sounds
        // load water data
        
        infile.close();
    } catch (std::ifstream::failure e) {
        Console_PrintLogString("Failed to load " + file_name + ": " + strerror(errno));
        std::cerr << "Exception opening/reading/closing file\n";
    }
}
