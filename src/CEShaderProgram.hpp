#pragma once

#define GLFW_INCLUDE_GLCOREARB
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <map>

namespace CE {
    class ShaderProgram
    {
    private:
        enum Type {
            VERTEX = GL_VERTEX_SHADER,
            FRAGMENT = GL_FRAGMENT_SHADER
        };
        
        GLuint m_program;
        std::map<Type, GLuint> m_shaders;
        
        std::string load(const std::string& file_name);
        GLuint create(const std::string& content, Type type);
        void check(GLuint shader, GLuint flag, bool is_program, const std::string& error_message);
    public:
        ShaderProgram(const std::string& file_name);
        
        void bind();
        virtual void updateUniforms();
        GLuint getProgram() const;
        
        virtual ~ShaderProgram();
    };
};
