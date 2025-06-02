#include <SFML/System.hpp>

#ifdef _WIN32
    const char* player_sheet = "../../../resources/full_sheet_outlined_3.png";
    const char* ghost_sheet = "../../../resources/ghost sheet outlined-white.png";
    const char* heart_sprite = "../../../resources/heart.png";
    const char* font_path = "../../../resources/Silkscreen-Regular.ttf";
    const char* animated_heart = "../../../resources/heartsheet.png";
#else
    const char* player_sheet = "../../resources/full_sheet_outlined_3.png";
    const char* ghost_sheet = "../../resources/ghost sheet outlined-white.png";
    const char* heart_sprite = "../../resources/heart.png";
    const char* font_path = "../../resources/Silkscreen-Regular.ttf";
    const char* animated_heart = "../../resources/heartsheet.png";
#endif

const unsigned window_width = 1280;
const unsigned window_height = 720;
const sf::Vector2f player_scale = {10, 10};
const unsigned h_sheet = 4;
const unsigned v_sheet = 8;
const sf::Vector2i player_sprite_size = {14, 15};
const sf::Vector2i ghost_sprite_size = {19, 21};
const sf::Vector2i heart_sprite_size = {16, 16};
const float player_speed = 500;
const float animation_fps_period = 1.0/5.0;