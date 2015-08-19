/*
 * World level class controls asset loading and score keeping.
 *
 * Additional responsibilities:
 * + Control HUD
 * + Enable reset/restart
 * + Control timing
 * + Character management
 * + Shared asset management
 */

#ifndef __CE_Character_Lab__CE_Level__
#define __CE_Character_Lab__CE_Level__

#include <stdio.h>
#include <memory>
#include <string>
#include <vector>

// Forward declarations
class CE_Renderer;
class C2MapFile;
class C2MapRscFile;

class C2CarFilePreloader;

class CE_HumanPlayer; // state representation for real "human" players (which may be representing non-human characters). Links to score, weapons, accessories, and current character
class C2Character; // state representations for all characters, which may or may not be controlled by a human player (AI_LocalUser or AI_RemoteUser)

class CE_PlayerHud;

class CE_Level
{
private:
  std::unique_ptr<CE_Renderer> m_renderer; // Base class controls rendering capabilities
  std::unique_ptr<C2MapFile> m_map_file;
  std::unique_ptr<C2MapRscFile> m_map_resource_file;
  
  std::unique_ptr<C2CarFilePreloader> m_car_files; // cache car files for this level

  std::vector< std::unique_ptr<CE_HumanPlayer> > m_humans; // supports multiple humans. We track them because we want to be able to display scores
  std::vector< std::unique_ptr<C2Character> > m_characters;
  
  bool m_is_loaded = false; // has the game level been loaded and characters placed?
  
  /*
   * This enables us to attach the camera to follow different players. Ultimately, the AI will decide how to
   * process input. Example: AI_LocalUser processes all input and updates the CE_Human state. AI_RemoteUser only processes
   * specific commands (change 3rd person view, stop following, etc).
   */
  CE_HumanPlayer* m_tracking_player; // Current player at whom to direct user input and render current view.
  std::unique_ptr<CE_PlayerHud> m_player_hud; // tracks HUD state for the current tracking player
  
  void place_map_characters(); // TODO: map should decide what to place via LUA script. This includes ambient and unlicensed characters (allows different ambients + unlicnesed depending on map)
  void place_selected_huntable_characters(); //TODO: Need param to decide what to place?

public:
  CE_Level(const std::string& map_file_name, const std::string& map_resource_file_name ); // Supports loading a map based on old file format
  ~CE_Level();
  
  void setup(); // Init the score and place characters. TODO: Use params to place huntables and set player state
  void reload(); // release characters, reload map (if needed), and re-place characters (by running setup() again)

  void processUserInput(); // Process keyboard input. Process level commands first (exit, restart, etc), then directs towards current tracking player
  void update(int time_delta); // Updates AI and game state
};

#endif /* defined(__CE_Character_Lab__CE_Level__) */
