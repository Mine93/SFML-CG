#include <SFML/Graphics.hpp>
#include <cmath>

const unsigned window_width = 1280;
const unsigned window_height = 720;
const sf::Vector2f player_scale = {10, 10};
const char* player_sheet = "resources/full_sheet_outlined_3.png";
const char* ghost_sheet = "resources/ghost sheet outlined-white.png";
const unsigned h_sheet = 4;
const unsigned v_sheet = 8;
const sf::Vector2i player_sprite_size = {14, 15};
const sf::Vector2i ghost_sprite_size = {19, 21};
const float player_speed = 500;
const float animation_fps_period = 1.0/5.0;

float dist(sf::Vector2f p1, sf::Vector2f p2){
    return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

sf::Angle angle(sf::Vector2f p1, sf::Vector2f p2){
    return sf::radians(atan2(p2.y - p1.y, p2.x - p1.x));
}

struct After_Image{
    sf::Vector2f position;
    sf::Sprite* sprite;

    After_Image(){}
    After_Image(sf::Vector2f o, sf::Vector2f s, sf::Texture& t){
        sprite = new sf::Sprite(t);
        sprite->setScale(s);
        sprite->setOrigin(o);
        sprite->setColor(sf::Color(127, 127, 127));
    }

    void set_start(sf::IntRect r, sf::Vector2f p){
        sprite->setTextureRect(r);
        position = p;
        sprite->setPosition(position);
    }

    void draw(sf::RenderWindow& window){
        window.draw(*sprite);
    }
};

struct Animation_Updater{
    float time_elapsed;
    int progression;
    float period;
    int max;
    sf::Vector2i size;

    Animation_Updater(){}
    Animation_Updater(float p, int m, sf::Vector2i s){
        time_elapsed = 0;
        period = p;
        progression = 0;
        max = m;
        size = s;
    }

    void update(float delta){
        time_elapsed += delta;
        if(time_elapsed >= period){
            time_elapsed -= period;
            progression = (progression + 1) % max;
        }
    }

    sf::IntRect get_sprite(int direction, bool moving){
        return sf::IntRect({{progression * size.x, (direction + moving * 4) * size.y}, size});
    }
};

struct Player{
    sf::Vector2f position;
    sf::Vector2f origin;
    sf::Vector2i size;
    sf::Vector2f scale;
    sf::Texture texture;
    sf::Sprite* sprite;

    float speed;
    int sprite_direction;
    bool moving;
    bool dashing;

    Animation_Updater anim_upd;
    After_Image aftr_img;

    Player(){
        position = {window_width / 2, window_height / 2};
        scale = player_scale;
        size = player_sprite_size;
        origin = {static_cast<float>(size.x / 2), static_cast<float>(size.y / 2)};

        if(!texture.loadFromFile(player_sheet))
            exit(-1);
        sprite = new sf::Sprite(texture);
        sprite->setOrigin(origin);
        sprite->setScale(scale);

        aftr_img = After_Image(origin, scale, texture);
        anim_upd = Animation_Updater(animation_fps_period, h_sheet, size);

        speed = player_speed;
        sprite_direction = 2;
        moving = false;
        dashing = false;
    }

    void draw(sf::RenderWindow& window){
        if(dashing){
            draw_line(window);
            aftr_img.draw(window);
        }
        sprite->setTextureRect(anim_upd.get_sprite(sprite_direction, moving));
        sprite->setPosition(position);

        window.draw(*sprite);
    }

    void update(float delta, sf::Vector2f movement){
        move_and_collide(movement, delta);

        calculate_direction(movement);
        if(movement.length() != 0 && moving == false)
            moving = true;
        else if(movement.length() == 0 && moving == true)
            moving = false;

        anim_upd.update(delta);
    }

    void calculate_direction(sf::Vector2f vec){
        unsigned tmp = sprite_direction;
        if(vec.x > 0)
            tmp = 0;
        else if(vec.x < 0)
            tmp = 1;
        else if(vec.y > 0)
            tmp = 2;
        else if(vec.y < 0)
            tmp = 3;

        if(tmp != sprite_direction)
            sprite_direction = tmp;
    }

    void start_dash(){
        if(dashing) return;

        dashing = true;
        aftr_img.set_start(anim_upd.get_sprite(sprite_direction, moving), position);
    }

    void stop_dash(){
        dashing = false;
        position = aftr_img.position;
    }

    void draw_line(sf::RenderWindow& window){
        sf::RectangleShape line({dist(position, aftr_img.position), 10});
        line.setFillColor(sf::Color::White);
        line.setOrigin({0, 5});
        line.setPosition(position);
        line.setRotation(angle(position, aftr_img.position));

        window.draw(line);
    }

    void move_and_collide(sf::Vector2f movement, float delta){
        if(position.x >= window_width - (size.x / 2 * scale.x) && movement.x > 0)
            movement.x = 0;
        else if(position.x <= (size.x / 2 * scale.x) && movement.x < 0)
            movement.x = 0;
        if(position.y >= window_height - (size.y / 2 * scale.y) && movement.y > 0)
            movement.y = 0;
        else if(position.y <= (size.y / 2 * scale.y) && movement.y < 0)
            movement.y = 0;

        position += movement * delta * speed;
    }

    bool is_inside(sf::Vector2f point){
        return (point.x >= position.x - (size.x / 2 * scale.x)) &&
               (point.x <= position.x + (size.x / 2 * scale.x)) &&
               (point.y >= position.y - (size.y / 2 * scale.y)) &&
               (point.y <= position.y + (size.y / 2 * scale.y));
    }

    void hit(){
        sprite->setColor(sf::Color::Red);
    }
};

struct Ghost{
    sf::Vector2f position;
    sf::Vector2f origin;
    sf::Vector2i size;
    sf::Vector2f scale;
    sf::Texture texture;
    sf::Sprite* sprite;

    Animation_Updater anim_upd;
    float speed;

    Ghost* prev_ghost;
    Ghost* next_ghost;

    Ghost(){
        position = {window_width / 3, window_height / 3};
        origin = {static_cast<float>(ghost_sprite_size.x / 2), static_cast<float>(ghost_sprite_size.y / 2)};
        size = ghost_sprite_size;
        scale = player_scale;
        speed = 100;

        if(!texture.loadFromFile(ghost_sheet))
            exit(-1);
        sprite = new sf::Sprite(texture);
        sprite->setOrigin(origin);
        sprite->setScale(scale);

        anim_upd = Animation_Updater(animation_fps_period, h_sheet, size);

        prev_ghost = nullptr;
        next_ghost = nullptr;
    }

    void draw(sf::RenderWindow& window){
        sprite->setPosition(position);
        sprite->setTextureRect(anim_upd.get_sprite(0, false));

        window.draw(*sprite);
    }

    void update(float delta, Player& p){
        position += sf::Vector2f(speed, angle(position, p.position)) * delta;
        anim_upd.update(delta);
        if(player_hit(p))
            p.hit();
    }

    bool player_hit(Player &p){
        return (p.is_inside({position.x - (size.x / 2 * scale.x), position.y})) ||
               (p.is_inside({position.x + (size.x / 2 * scale.x), position.y})) ||
               (p.is_inside({position.x, position.y + (size.y / 2 * scale.y)})) ||
               (p.is_inside({position.x, position.y - (size.y / 2 * scale.y)}));
    }
};

struct State{
    Player player;
    Ghost ghost;

    bool move_up;
    bool move_left;
    bool move_down;
    bool move_right;

    State(){
        move_up = false;
        move_left = false;
        move_down = false;
        move_right = false;
    }

    void draw(sf::RenderWindow& window){
        player.draw(window);
        ghost.draw(window);
    }

    void update(float delta){
        sf::Vector2f movement(move_right - move_left, move_down - move_up);
        if(movement.length() != 0)
            movement = movement.normalized();
        player.update(delta, movement);
        ghost.update(delta, player);
    }
};

void handle_close (sf::RenderWindow& window){
    window.close();
}

void handle_resize (const sf::Event::Resized& resized, sf::RenderWindow& window){
    sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized.size));
    window.setView(sf::View(visibleArea));
}

void handle(const sf::Event::KeyPressed &KeyPressed, State &state){
    switch(KeyPressed.code){
        case sf::Keyboard::Key::W:
            state.move_up = true;
            break;
        case sf::Keyboard::Key::A:
            state.move_left = true;
            break;
        case sf::Keyboard::Key::S:
            state.move_down = true;
            break;
        case sf::Keyboard::Key::D:
            state.move_right = true;
            break;
    }

    if(KeyPressed.code == sf::Keyboard::Key::LShift)
        state.player.start_dash();
}

void handle(const sf::Event::KeyReleased &KeyReleased, State &state){
    switch(KeyReleased.code){
        case sf::Keyboard::Key::W:
            state.move_up = false;
            break;
        case sf::Keyboard::Key::A:
            state.move_left = false;
            break;
        case sf::Keyboard::Key::S:
            state.move_down = false;
            break;
        case sf::Keyboard::Key::D:
            state.move_right = false;
            break;
    }

    if(KeyReleased.code == sf::Keyboard::Key::LShift)
        state.player.stop_dash();
}

void handle(const sf::Event::FocusGained, State &state){

}

void handle(const sf::Event::FocusLost, State &state){

}

template <typename T>
void handle(const T& event, State& state){}

int main(){
    sf::RenderWindow window (sf::VideoMode ({window_width, window_height}), "Dasher");
    window.setVerticalSyncEnabled(true);
    //window.setFramerateLimit(1);

    State state;
    sf::Clock delta;

    while (window.isOpen()){
        window.handleEvents([&window](const sf::Event::Closed&){handle_close(window);},
                            [&window](const sf::Event::Resized& event){handle_resize(event, window);},
                            [&state] (const auto& event){handle(event, state);});

        state.update(delta.restart().asSeconds());

        window.clear(sf::Color::Black);
        state.draw(window);
        window.display();
    }
}