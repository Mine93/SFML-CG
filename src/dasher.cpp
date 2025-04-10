#include <SFML/Graphics.hpp>
#include <cmath>

const unsigned window_width = 1280;
const unsigned window_height = 720;
const sf::Vector2f player_scale = {10, 10};
const char* player_sheet = "resources/full_sheet_outlined_3.png";
const unsigned h_sheet = 4;
const unsigned v_sheet = 8;
const sf::Vector2i sprite_size = {14, 15};
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
    sf::Vector2f origin;
    sf::Vector2f scale;
    sf::Texture texture;
    sf::IntRect sprite_rect;

    After_Image(sf::Vector2f p, sf::Vector2f o, sf::Vector2f s, sf::Texture t, sf::IntRect r){
        position = p;
        origin = o;
        scale = s;
        texture = t;
        sprite_rect = r;
    }

    void draw(sf::RenderWindow& window){
        sf::Sprite sprite(texture);
        sprite.setTextureRect(sprite_rect);
        sprite.setScale(scale);
        sprite.setOrigin(origin);
        sprite.setPosition(position);
        sprite.setColor(sf::Color(127, 127, 127));

        window.draw(sprite);
    }
};

struct Player{
    sf::Vector2f position;
    sf::Vector2f origin;
    sf::Vector2i size;
    sf::Vector2f scale;
    sf::Texture texture;
    float speed;
    float time_elapsed;
    int sprite_progression;
    int sprite_direction;
    bool moving;
    bool dashing;
    After_Image* aftr_img;

    Player(){
        position = {window_width / 2, window_height / 2};
        scale = player_scale;
        size = sprite_size;
        origin = {static_cast<float>(size.x / 2), static_cast<float>(size.y / 2)};
        if(!texture.loadFromFile(player_sheet))
            exit(-1);
        speed = player_speed;
        time_elapsed = 0;
        sprite_progression = 0;
        sprite_direction = 2;
        moving = false;
        dashing = false;
    }

    void draw(sf::RenderWindow& window){
        if(dashing){
            draw_line(window);
            aftr_img->draw(window);
        }

        sf::Sprite sprite(texture);
        sprite.setTextureRect(get_sprite());
        sprite.setScale(scale);
        sprite.setOrigin(origin);
        sprite.setPosition(position);

        window.draw(sprite);
    }

    void update(float delta, sf::Vector2f movement){
        position += movement * delta * speed;

        calculate_direction(movement);
        if(movement.length() != 0 && moving == false)
            moving = true;
        else if(movement.length() == 0 && moving == true)
            moving = false;

        update_animation(delta);
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

    void update_animation(float delta){
        time_elapsed += delta;
        if(time_elapsed >= animation_fps_period){
            time_elapsed -= animation_fps_period;
            sprite_progression = ++sprite_progression % h_sheet;
        }
    }

    sf::IntRect get_sprite(){
        return sf::IntRect({{sprite_progression * size.x, (sprite_direction + moving * 4) * size.y}, size});
    }

    void start_dash(){
        if(dashing) return;

        dashing = true;
        aftr_img = new After_Image(position, origin, scale, texture, get_sprite());
    }

    void stop_dash(){
        dashing = false;
        position = aftr_img->position;
        free(aftr_img);
    }

    void draw_line(sf::RenderWindow& window){
        sf::RectangleShape line({dist(position, aftr_img->position), 10});
        line.setFillColor(sf::Color::White);
        line.setOrigin({0, 5});
        line.setPosition(position);
        line.setRotation(angle(position, aftr_img->position));

        window.draw(line);
    }
};



struct State{
    Player player;

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
    }

    void update(float delta){
        sf::Vector2f movement(move_right - move_left, move_down - move_up);
        if(movement.length() != 0)
            movement = movement.normalized();
        player.update(delta, movement);
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
        // events
        window.handleEvents([&window](const sf::Event::Closed&){handle_close(window);},
                            [&window](const sf::Event::Resized& event){handle_resize(event, window);},
                            [&state] (const auto& event){handle(event, state);});

        // display
        state.update(delta.restart().asSeconds());

        window.clear(sf::Color::Black);
        state.draw(window);
        window.display();
    }
}
