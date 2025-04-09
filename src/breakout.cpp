#include <SFML/Graphics.hpp>
#include "textures.hpp"



//////////////////////
// Initial defaults //
//////////////////////

// window
const char* window_title = "breakout";
const unsigned window_width = 800;
const unsigned window_height = 600;
const float max_frame_rate = 60;
const float ball_radius = 10.0;
const sf::Vector2f paddle_size = {100.0, 16.0};
const float ball_speed = 300;
const sf::Angle ball_start_angle = sf::degrees(-70);
const float paddle_speed = 400;
const sf::Angle left_max_angle = sf::degrees(-170);
const sf::Vector2f wall_position = {50, 100};
const sf::Vector2f wall_dimesion = {700, 150};

sf::Angle reflect_horizontal(sf::Angle a){
    sf::Vector2f tmp(1.0, a);
    tmp.x = -tmp.x;
    a = tmp.angle();
    return a;
}

struct Ball{
    float radius;
    sf::Vector2f position;
    sf::Texture texture;
    float speed;
    sf::Angle direction;

    Ball(){
        radius = ball_radius;
        position = {(window_width / 2) - ball_radius, window_height - paddle_size.y - ball_radius * 2};
        texture = sf::Texture(ball_png, ball_png_len);
        speed = ball_speed;
        direction = ball_start_angle;
    }

    void draw(sf::RenderWindow& window){
        sf::CircleShape ball(ball_radius, 30.f);
        ball.setPosition(position);
        ball.setTexture(&texture);
        window.draw(ball);
    }

    void move(float delta){
        position += sf::Vector2f(speed * delta, direction);
    }
};

struct Paddle{
    sf::Vector2f size;
    sf::Vector2f position;
    sf::Texture texture;
    float speed;

    Paddle(){
        size = paddle_size;
        position = {(window_width / 2) - (paddle_size.x / 2), window_height - paddle_size.y};
        texture = sf::Texture(paddle_png, paddle_png_len);
        speed = paddle_speed;
    }

    void draw(sf::RenderWindow& window){
        sf::RectangleShape paddle(size);
        paddle.setPosition(position);
        paddle.setTexture(&texture);
        window.draw(paddle);
    }

    void move_left(float delta){
        if(position.x > 0)
            position.x -= delta * speed;
    }

    void move_right(float delta){
        if(position.x < window_width - size.x)
            position.x += delta * speed;
    }

    bool hit(Ball b){
        if(b.position.y > window_height - size.y - b.radius * 2)
            if((b.position.x >= position.x) && (b.position.x <= position.x + size.x))
                return true;
        return false;
    }

    void strike(Ball& b){
        if(!hit(b)) return;

        if(b.position.y > window_height - size.y / 2){
            b.direction = -b.direction;
        }

        float distance = (b.position.x + b.radius) - (position.x + size.x / 2);
        distance /= size.x + b.radius * 2;
        distance += 0.5;
        b.direction = sf::radians(linear_interpolation(left_max_angle.asRadians(), reflect_horizontal(left_max_angle).asRadians(), distance));
    }

    float linear_interpolation(float v0, float v1, float t){
        return (1-t) * v0 + t * v1;
    }
};

struct Block{
    sf::Vector2f position;
    sf::Vector2f size;
    bool intact;

    Block(sf::Vector2f p, sf::Vector2f s){
        position = p;
        size = s;
        intact = true;
    }

    void draw(sf::RenderWindow& window){
        if(intact){
            sf::RectangleShape brick(size);
            brick.setPosition(position);
            brick.setOutlineThickness(-3);
            brick.setFillColor(sf::Color::Yellow);
            brick.setOutlineColor(sf::Color::Red);
            window.draw(brick);
        }
    }

    bool is_inside(sf::Vector2f p){
        if(p.x >=  position.x && p.x <= position.x + size.x)
            if(p.y >=  position.y && p.y <= position.y + size.y)
                return true;
        return false;
    }

    void hit(Ball& b){
        if(!intact) return;

        sf::Vector2f center = {b.position.x + b.radius, b.position.y + b.radius};
        sf::Vector2f north = {center.x, center.y - b.radius};
        sf::Vector2f east = {center.x + b.radius, center.y};
        sf::Vector2f south = {center.x + b.radius, center.y};
        sf::Vector2f west = {center.x, center.y + b.radius};

        sf::Vector2f movement(1.0, b.direction);
        if(is_inside(north) && movement.y < 0){
            b.direction = -b.direction;
            intact = false;
        }
        if(is_inside(east) && movement.x > 0){
            b.direction = reflect_horizontal(b.direction);
            intact = false;
        }
        if(is_inside(south) && movement.y > 0){
            b.direction = -b.direction;
            intact = false;
        }
        if(is_inside(west) && movement.x < 0){
            b.direction = reflect_horizontal(b.direction);
            intact = false;
        }
    }
};

struct Wall{
    std::vector<Block> bricks;

    Wall(){
        const unsigned h_n_bricks = 10;
        const unsigned v_n_bricks = 5;
        sf::Vector2f brick_size = {wall_dimesion.x / 10, wall_dimesion.y / 5};
        for(size_t i = 0; i < v_n_bricks; i++){
            for(size_t j = 0; j < h_n_bricks; j++){
                bricks.push_back(Block({wall_position.x + static_cast<float>(j) * brick_size.x, wall_position.y + static_cast<float>(i) * brick_size.y}, brick_size));
            }
        }
    }

    void draw(sf::RenderWindow& window){
        for(Block brick: bricks){
            brick.draw(window);
        }
    }

    void hit(Ball& b){
        for(Block& brick: bricks){
            brick.hit(b);
        }
    }
};


struct State{
    Ball ball;
    Paddle paddle;
    Wall wall;

    bool pause;
    bool focused;
    
    bool move_left;
    bool move_right;

    State(){
        pause = true;
        focused = false;
        move_left = false;
        move_right = false;
    }

    void draw(sf::RenderWindow& window){
        ball.draw(window);
        paddle.draw(window);
        wall.draw(window);
    }

    void update(float delta){
        if(!pause){
            ball.move(delta);
            if(move_left)
                paddle.move_left(delta);
            else if(move_right)
                paddle.move_right(delta);
            collisions();
        }
    }

    void field_limits(){
        sf::Vector2f movement(1.0, ball.direction);

        if((ball.position.x <= 0)){
            if(movement.x < 0)
                ball.direction = reflect_horizontal(ball.direction);
        }
        else if(ball.position.x >= (window_width - ball_radius * 2)){
            if(movement.x > 0)
                ball.direction = reflect_horizontal(ball.direction);
        }

        if(ball.position.y <= 0){
            if(movement.y < 0)
                ball.direction = -ball.direction;
        }
        else if(ball.position.y >= (window_height - ball_radius * 2)){
            if(movement.y > 0)
                restart();
        }
    }

    void collisions(){
        field_limits();
        paddle.strike(ball);
        wall.hit(ball);
    }

    void restart(){
        ball = Ball();
        paddle = Paddle();
        wall = Wall();
        pause = true;
        focused = false;
        move_left = false;
        move_right = false;
    }
};

////////////
// Events //
////////////

void handle_close (sf::RenderWindow& window)
{
    window.close();
}
void handle_resize (const sf::Event::Resized& resized, sf::RenderWindow& window)
{   // constrain aspect ratio and map always the same portion of the world
    float aspect = static_cast<float>(window_width)/static_cast<float>(window_height);
    sf::Vector2u ws = resized.size;
    float new_aspect = static_cast<float>(ws.x)/static_cast<float>(ws.y);
    if (new_aspect < aspect)
        ws = {ws.x,static_cast<unsigned>(ws.x/aspect)};
        else
        ws = {static_cast<unsigned>(ws.y*aspect),ws.y};
    window.setSize(ws);
}
void handle(const sf::Event::KeyPressed &KeyPressed, State &state){
    if(KeyPressed.code == sf::Keyboard::Key::Space){
        state.pause = !state.pause;
    }
    if(KeyPressed.code == sf::Keyboard::Key::Left)
        state.move_left = true;
    else if(KeyPressed.code == sf::Keyboard::Key::Right)
        state.move_right = true;
}
void handle(const sf::Event::KeyReleased &KeyReleased, State &state){
    if(KeyReleased.code == sf::Keyboard::Key::Left)
        state.move_left = false;
    else if(KeyReleased.code == sf::Keyboard::Key::Right)
        state.move_right = false;
}
template <typename T>
void handle(const T& event, State& state){}
void handle(const sf::Event::FocusGained, State &state){
    state.focused = true;
}
void handle(const sf::Event::FocusLost, State &state){
    state.focused = false;
    state.pause = true;
}



//////////
// Loop //
//////////

int main()
{
    sf::RenderWindow window (sf::VideoMode ({window_width, window_height}), window_title);
    //window.setFramerateLimit (max_frame_rate);
    window.setVerticalSyncEnabled(true);
    window.setMinimumSize(window.getSize());

    State state;
    sf::Clock delta;

    while (window.isOpen())
    {
        // events
        window.handleEvents (
                             [&window](const sf::Event::Closed&) { handle_close (window); },
                             [&window](const sf::Event::Resized& event) { handle_resize (event, window); },
                             [&state] (const auto& event) { handle (event, state); }
        );

        // display
        state.update(delta.restart().asSeconds());

        window.clear (sf::Color::Black);
        state.draw(window);
        window.display ();
    }
}
