#include "entities.hpp"
#include "defaults.hpp"

#ifndef _WIN32
    #include <cmath>
#endif

float dist(sf::Vector2f p1, sf::Vector2f p2){
    return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

sf::Angle angle(sf::Vector2f p1, sf::Vector2f p2){
    return sf::radians(atan2(p2.y - p1.y, p2.x - p1.x));
}

bool edgeIntersects(const sf::Vector2f& A, const sf::Vector2f& B, const sf::Vector2f& C, const sf::Vector2f& D) {
    float det = (B.x - A.x) * (D.y - C.y) - (B.y - A.y) * (D.x - C.x);
    if (det == 0) return false;

    float t = ((C.x - A.x) * (D.y - C.y) - (C.y - A.y) * (D.x - C.x)) / det;
    float u = ((C.x - A.x) * (B.y - A.y) - (C.y - A.y) * (B.x - A.x)) / det;

    return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
}

Entity::Entity(sf::Vector2f position, sf::Vector2f origin, const sf::Vector2i sprite_size, const sf::Vector2f scale, const char* texture_path, const float animation_period, const unsigned n_frames, unsigned sprite_direction):
    position(position),
    origin(origin),
    sprite_size(sprite_size),
    scale(scale),
    size({sprite_size.x * scale.x, sprite_size.y * scale.y}),
    sprite(texture),
    anim(animation_period, n_frames, sprite_size),
    sprite_direction(sprite_direction),
    moving(false){
        if(!texture.loadFromFile(texture_path))
            exit(-1);
        sprite.setOrigin(origin);
        sprite.setScale(scale);
}

bool Entity::update(float delta){
    return anim.update(delta);
}

void Entity::draw(sf::RenderWindow& window){
    sprite.setPosition(position);
    sprite.setTextureRect(anim.get_sprite(sprite_direction, moving));

    window.draw(sprite);
}

Animation_Updater::Animation_Updater(float period, unsigned max, sf::Vector2i sprite_size):
        time_elapsed(0),
        progression(0),
        period(period),
        max(max),
        sprite_size(sprite_size){}

bool Animation_Updater::update(float delta){
    time_elapsed += delta;
    if(time_elapsed >= period){
        time_elapsed -= period;
        progression = (progression + 1) % max;
        return true;
    }
    return false;
}

void Animation_Updater::draw(sf::RenderWindow &window){}

sf::IntRect Animation_Updater::get_sprite(int direction, bool moving){
    return sf::IntRect({{progression * sprite_size.x, (direction + moving * 4) * sprite_size.y}, sprite_size});
}

//Player::Player(){}
Player::Player(bool directions[4]):
    Entity({window_width / 2, window_height / 2}, {static_cast<float>(player_sprite_size.x / 2), static_cast<float>(player_sprite_size.y / 2)}, player_sprite_size, player_scale, player_sheet, animation_fps_period, h_sheet, 2),
    speed(player_speed),
    dashing(false),
    invulnerable(false),
    dead(false),
    attack(false),
    prev_attack(false),
    success(false),
    health(3),
    inv_window(0),
    directions(directions),
    aftr(origin, scale, texture){}

//void Player::update(float delta){}
bool Player::update(float delta){
    if(dead) return true;

    Entity::update(delta);

    if(success){
        success = false;
        position = aftr.position;
    }

    sf::Vector2f movement(directions[0] - directions[1], directions[2] - directions[3]);
    if(movement.length() != 0)
        movement = movement.normalized();
    move_and_collide(movement, delta);

    if(attack){
        if(prev_attack){
            attack = false;
            prev_attack = false;
        }
        else
            prev_attack = true;
    }

    calculate_direction(movement);
    if(movement.length() != 0 && moving == false)
        moving = true;
    else if(movement.length() == 0 && moving == true)
        moving = false;


    if(invulnerable){
        inv_window += delta;
        if(inv_window >= 1.5){
            inv_window = 0;
            invulnerable = false;
            sprite.setColor(sf::Color::White);
        }
    }
    return false;
}

void Player::draw(sf::RenderWindow& window){
    if(dashing){
        draw_line(window);
        aftr.draw(window);
    }

    Entity::draw(window);
}

void Player::calculate_direction(sf::Vector2f vec){
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

void Player::start_dash(){
    if(dashing || dead) return;

    dashing = true;
    aftr.set_start(anim.get_sprite(sprite_direction, moving), position);
}

void Player::stop_dash(){
    if(!dashing || dead) return;

    dashing = false;
    attack = true;
}

void Player::draw_line(sf::RenderWindow& window){
    sf::RectangleShape line({dist(position, aftr.position), 10});
    line.setFillColor(sf::Color::White);
    line.setOrigin({0, 5});
    line.setPosition(position);
    line.setRotation(angle(position, aftr.position));

    window.draw(line);
}

void Player::move_and_collide(sf::Vector2f movement, float delta){
    if(position.x >= window_width - (sprite_size.x / 2 * scale.x) && movement.x > 0)
        movement.x = 0;
    else if(position.x <= (sprite_size.x / 2 * scale.x) && movement.x < 0)
        movement.x = 0;
    if(position.y >= window_height - (sprite_size.y / 2 * scale.y) && movement.y > 0)
        movement.y = 0;
    else if(position.y <= (sprite_size.y / 2 * scale.y) && movement.y < 0)
        movement.y = 0;

    position += movement * delta * speed;
}

void Player::hit(){
    if(invulnerable) return;

    if(--health == 0){
        dead = true;
        //sprite.setColor(sf::Color(127,127,127));
        sprite.setRotation(sf::degrees(90));
        dashing = false;
    }
    else
        sprite.setColor(sf::Color::Red);
    invulnerable = true;
}

//After_Image::After_Image(){}
After_Image::After_Image(sf::Vector2f origin, sf::Vector2f size, sf::Texture& texture):
    sprite(texture){
        sprite.setScale(size);
        sprite.setOrigin(origin);
        sprite.setColor(sf::Color(127, 127, 127));
}

bool After_Image::update(float delta){return false;}

void After_Image::draw(sf::RenderWindow& window){
    window.draw(sprite);
}

void After_Image::set_start(sf::IntRect rect, sf::Vector2f position){
    sprite.setTextureRect(rect);
    this->position = position;
    sprite.setPosition(position);
}

//Ghost::Ghost(){}
Ghost::Ghost(sf::Vector2f position,Player* player):
    Entity(position, {static_cast<float>(ghost_sprite_size.x / 2), static_cast<float>(ghost_sprite_size.y / 2)}, ghost_sprite_size, player_scale, ghost_sheet, animation_fps_period, h_sheet, 0),
    speed(100),
    player(player){}

//void Ghost::update(float delta){}
bool Ghost::update(float delta){
    Entity::update(delta);

    position += sf::Vector2f(speed, angle(position, player->position)) * delta;

    if(player_hit(player->position))
        player->hit();

    if(player_hurt()){
        player->success = true;
        return true;
    }

    return false;
}

void Ghost::draw(sf::RenderWindow& window){
    Entity::draw(window);
}

bool Ghost::player_hit(sf::Vector2f p_position){
    /*return (((position.x + (size.x / 2 * scale.x)) >= (p.position.x - (p.size.x / 2 * p.scale.x))) &&
            ((position.x - (size.x / 2 * scale.x)) <= (p.position.x + (p.size.x / 2 * p.scale.x))) &&
            ((position.y + (size.y / 2 * scale.y)) >= (p.position.y - (p.size.y / 2 * p.scale.y))) &&
            ((position.y - (size.y / 2 * scale.y)) <= (p.position.y + (p.size.y / 2 * p.scale.y))));*/
    return dist(position, p_position) < 150;
}

bool Ghost::player_hurt(){
    if(!player->attack) return false;
    return (edgeIntersects(player->position, player->aftr.position, {position.x - (sprite_size.x / 2 * scale.x), position.y - (sprite_size.y / 2 * scale.y)}, {position.x + (sprite_size.x / 2 * scale.x), position.y - (sprite_size.y / 2 * scale.y)}) ||
            edgeIntersects(player->position, player->aftr.position, {position.x + (sprite_size.x / 2 * scale.x), position.y - (sprite_size.y / 2 * scale.y)}, {position.x + (sprite_size.x / 2 * scale.x), position.y + (sprite_size.y / 2 * scale.y)}) ||
            edgeIntersects(player->position, player->aftr.position, {position.x + (sprite_size.x / 2 * scale.x), position.y + (sprite_size.y / 2 * scale.y)}, {position.x - (sprite_size.x / 2 * scale.x), position.y + (sprite_size.y / 2 * scale.y)}) ||
            edgeIntersects(player->position, player->aftr.position, {position.x - (sprite_size.x / 2 * scale.x), position.y + (sprite_size.y / 2 * scale.y)}, {position.x - (sprite_size.x / 2 * scale.x), position.y - (sprite_size.y / 2 * scale.y)}));
}

Horde::Horde(Player* player):
        time_elapsed(0),
        player(player){
            srand(time(0));
}

//void Horde::update(float delta){}
bool Horde::update(float delta){
    bool ret = false;
    time_elapsed += delta;
    if(time_elapsed >= 5){
        time_elapsed -= 5;
        horde.push_back(new Ghost(sf::Vector2f(window_width / 2, window_height / 2) + sf::Vector2f(700, sf::degrees(rand() % 360)), player));
        ret = true;
    }

    std::list<Ghost*>::iterator g = horde.begin();
    while(g != horde.end()){
        if((*g)->update(delta)){
            free(*g);
            g = horde.erase(g);
        }
        else
            g++;
    }
    return ret;
}

void Horde::draw(sf::RenderWindow& window){
    for(Ghost* g: horde)
        g->draw(window);
}

State::State():
    player(directions),
    horde(&player){
        if(!heart_texture.loadFromFile(heart_sprite))
            exit(-1);
}

bool State::update(float delta){
    if(player.update(delta))
        return true;
    horde.update(delta);
    return false;
}

void State::draw(sf::RenderWindow& window){
    horde.draw(window);
    player.draw(window);
    draw_health(window);
}

void State::draw_health(sf::RenderWindow& window){
    sf::Sprite heart(heart_texture);
    heart.setScale(player_scale);

    switch(player.health){
        case 3:
            heart.setPosition({310, 10});
            window.draw(heart);
        case 2:
            heart.setPosition({160, 10});
            window.draw(heart);
        case 1:
            heart.setPosition({10, 10});
            window.draw(heart);
    }
}