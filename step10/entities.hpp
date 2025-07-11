#include <SFML/Graphics.hpp>

#ifndef _WIN32
    #include <list>
#endif

struct Updatable{
    virtual bool update(float delta) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
};

struct Animation_Updater: Updatable{
    float time_elapsed;
    int progression;
    float period;
    unsigned max;
    sf::Vector2i sprite_size;

    Animation_Updater(float period, unsigned max, sf::Vector2i sprite_size);

    bool update(float delta) override;
    void draw(sf::RenderWindow& window) override;

    sf::IntRect get_sprite(int direction, bool moving);
};

struct Entity: Updatable{
    sf::Vector2f position;
    sf::Vector2f origin;
    sf::Vector2i sprite_size;
    sf::Vector2f scale;
    sf::Vector2f size;
    sf::Texture texture;
    sf::Sprite sprite;
    Animation_Updater anim;
    unsigned sprite_direction;
    bool moving;

    Entity(sf::Vector2f position, sf::Vector2f origin, const sf::Vector2i sprite_size, const sf::Vector2f scale, const char* texture_path, const float animation_period, const unsigned n_frames, unsigned sprite_direction);

    bool update(float delta) override;
    void draw(sf::RenderWindow& window) override;
};

struct After_Image: Updatable{
    sf::Vector2f position;
    sf::Sprite sprite;
    unsigned sprite_direction;

    After_Image(sf::Vector2f origin, sf::Vector2f size, sf::Texture& texture);

    bool update(float delta) override;
    void draw(sf::RenderWindow& window) override;

    void set_start(sf::IntRect rect, sf::Vector2f position, unsigned sprite_direction);
};

struct Player: Entity{
    float speed;
    bool dashing;
    bool invulnerable;
    float inv_window;
    float fail_window;
    bool dead;
    bool attack;
    bool prev_attack;
    bool success;
    bool fail;
    After_Image aftr;
    unsigned health;
    bool* directions;

    Player(bool directions[4]);

    bool update(float delta) override;
    void draw(sf::RenderWindow& window) override;

    void calculate_direction(sf::Vector2f dir);
    void start_dash();
    void stop_dash();
    void draw_line(sf::RenderWindow& window);
    void move_and_collide(sf::Vector2f direction, float delta);
    void hit();
    void successful_dash();
    void heal();
    void draw_fail_bar(sf::RenderWindow& window);
};

struct Ghost: Entity{
    float speed;
    Player* player;

    Ghost(sf::Vector2f position, Player* player);

    bool update(float delta) override;
    void draw(sf::RenderWindow& window) override;

    bool player_hit(sf::Vector2f p_position);
    bool player_hurt();
};

struct Heart: Updatable{
    sf::Vector2f position;
    sf::Texture texture;
    sf::Sprite sprite;
    Player* player;

    Heart(Player* player, sf::Vector2f position);
    
    bool update(float delta) override;
    void draw(sf::RenderWindow& window) override;
};

struct Horde: Updatable{
    std::list<Ghost*> horde;
    std::list<Heart*> hearts;
    float time_elapsed;
    unsigned long long score;
    Player* player;

    Horde(Player* player);

    bool update(float delta) override;
    void draw(sf::RenderWindow& window) override;

    bool spawn_enemies(float delta);
    bool spawn_hearts(sf::Vector2f position);
    void update_horde(float delta);
    void update_hearts();
};

struct State: Updatable{
    Player player;
    Horde horde;
    bool directions[4] = {false, false, false, false};
    sf::Texture heart_texture;

    State();

    bool update(float delta) override;
    void draw(sf::RenderWindow& window) override;

    void draw_health(sf::RenderWindow& window);
    void display_score(sf::RenderWindow& window);
};