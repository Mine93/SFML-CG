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

bool edge_intersects(const sf::Vector2f& a, const sf::Vector2f& b, const sf::Vector2f& c, const sf::Vector2f& d) {
    float det = (b.x - a.x) * (d.y - c.y) - (b.y - a.y) * (d.x - c.x);
    if (det == 0) return false;

    float t = ((c.x - a.x) * (d.y - c.y) - (c.y - a.y) * (d.x - c.x)) / det;
    float u = ((c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x)) / det;

    return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
}

Entity::Entity(sf::Vector2f position, sf::Vector2f origin, const sf::Vector2i sprite_size, const sf::Vector2f scale, const float animation_period, const unsigned n_frames, unsigned sprite_direction, sf::Texture& texture):
    position(position),
    origin(origin),
    sprite_size(sprite_size),
    scale(scale),
    size(sf::Vector2f(sprite_size.x * scale.x, sprite_size.y * scale.y)),
    sprite(texture),
    anim(animation_period, n_frames, sprite_size),
    sprite_direction(sprite_direction),
    moving(false){
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
Player::Player(bool directions[4], sf::Texture& texture, sf::SoundBuffer& hit_buffer):
    Entity(sf::Vector2f(window_width / 2, window_height / 2), sf::Vector2f(player_sprite_size.x / 2, player_sprite_size.y / 2), player_sprite_size, player_scale, animation_fps_period, h_sheet, 2, texture),
    speed(player_speed),
    dashing(false),
    invulnerable(false),
    dead(false),
    attack(false),
    prev_attack(false),
    success(false),
    fail(false),
    health(3),
    inv_window(0),
    fail_window(0),
    directions(directions),
    aftr(origin, scale, texture),
    screen_size(window_width, window_height),
    hit_sound(hit_buffer){
        hit_sound.setVolume(volume);
}

//void Player::update(float delta){}
bool Player::update(float delta){
    if(dead) return true;

    Entity::update(delta);

    sf::Vector2f movement(directions[0] - directions[1], directions[2] - directions[3]);
    if(movement.length() != 0)
        movement = movement.normalized();
    move_and_collide(movement, delta);

    if(attack){
        if(prev_attack){
            attack = false;
            prev_attack = false;
            successful_dash();
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

    if(fail){
        fail_window += delta;
        if(fail_window >= 1){
            fail_window = 0;
            fail = false;
        }
    }
    return false;
}

void Player::draw(sf::RenderWindow& window){
    if(dashing){
        draw_line(window);
        aftr.draw(window);
    }

    if(fail && !dead)
        draw_fail_bar(window);

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
    if(dashing || dead || fail || attack) return;

    dashing = true;
    aftr.set_start(anim.get_sprite(sprite_direction, moving), position, sprite_direction);
}

void Player::stop_dash(){
    if(!dashing || dead || fail || attack) return;

    dashing = false;
    attack = true;
}

void Player::draw_line(sf::RenderWindow& window){
    sf::RectangleShape line({dist(position, aftr.position), scale.x});
    line.setFillColor(sf::Color::White);
    line.setOrigin({0, scale.x / 2});
    line.setPosition(position);
    line.setRotation(angle(position, aftr.position));

    window.draw(line);
}

void Player::move_and_collide(sf::Vector2f movement, float delta){
    if(position.x >= screen_size.x - (sprite_size.x / 2 * scale.x) - 20 && movement.x > 0)
        movement.x = 0;
    else if(position.x <= (sprite_size.x / 2 * scale.x) + 20 && movement.x < 0)
        movement.x = 0;
    if(position.y >= screen_size.y - (sprite_size.y / 2 * scale.y) - 30 && movement.y > 0)
        movement.y = 0;
    else if(position.y <= (sprite_size.y / 2 * scale.y) + 100 && movement.y < 0)
        movement.y = 0;

    position += movement * delta * speed;
}

void Player::hit(){
    if(invulnerable) return;

    hit_sound.play();

    if(--health == 0){
        dead = true;
        //sprite.setColor(sf::Color(127,127,127));
        sprite.setRotation(sf::degrees(90));
    }
    else
        sprite.setColor(sf::Color::Red);
    
    if(dashing){
        dashing = false;
        fail = true;
    }
    invulnerable = true;
}

void Player::successful_dash(){
    if(success){
        success = false;
        position = aftr.position;
        sprite_direction = aftr.sprite_direction;
    }
    else
        fail = true;
}

void Player::heal(){
    if(health < 3 && !dead)
        health++;
}

void Player::draw_fail_bar(sf::RenderWindow& window){
    sf::RectangleShape ext({150, 20});
    ext.setOrigin({75,0});
    ext.setFillColor(sf::Color::Black);
    ext.setOutlineThickness(-5);
    ext.setOutlineColor(sf::Color::White);
    ext.setPosition(position + sf::Vector2f(0, 100));
    window.draw(ext);

    sf::RectangleShape ent({(150 * fail_window), 20});
    ent.setOrigin({75,0});
    ent.setFillColor(sf::Color::White);
    ent.setPosition(position + sf::Vector2f(0, 100));
    window.draw(ent);
}

//After_Image::After_Image(){}
After_Image::After_Image(sf::Vector2f origin, sf::Vector2f size, sf::Texture& texture):
    sprite(texture),
    sprite_direction(0){
        sprite.setScale(size);
        sprite.setOrigin(origin);
        //sprite.setColor(sf::Color(127, 127, 127));
}

bool After_Image::update(float delta){return false;}

void After_Image::draw(sf::RenderWindow& window){
    window.draw(sprite);
}

void After_Image::set_start(sf::IntRect rect, sf::Vector2f position, unsigned sprite_direction){
    sprite.setTextureRect(rect);
    this->position = position;
    sprite.setPosition(position);
    this->sprite_direction = sprite_direction;
}

//Ghost::Ghost(){}
Ghost::Ghost(sf::Vector2f position,Player* player, sf::Texture& texture):
    Entity(position, sf::Vector2f(ghost_sprite_size.x / 2, ghost_sprite_size.y / 2), ghost_sprite_size, player_scale, animation_fps_period, h_sheet, 0, texture),
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
    return dist(position, p_position) < scale.x * sprite_size.x / 2 + player->scale.x * player->sprite_size.x / 2 - 10;
}

bool Ghost::player_hurt(){
    if(!player->attack) return false;
    return (edge_intersects(player->position, player->aftr.position, {position.x - (sprite_size.x / 2 * scale.x), position.y - (sprite_size.y / 2 * scale.y)}, {position.x + (sprite_size.x / 2 * scale.x), position.y - (sprite_size.y / 2 * scale.y)}) ||
            edge_intersects(player->position, player->aftr.position, {position.x + (sprite_size.x / 2 * scale.x), position.y - (sprite_size.y / 2 * scale.y)}, {position.x + (sprite_size.x / 2 * scale.x), position.y + (sprite_size.y / 2 * scale.y)}) ||
            edge_intersects(player->position, player->aftr.position, {position.x + (sprite_size.x / 2 * scale.x), position.y + (sprite_size.y / 2 * scale.y)}, {position.x - (sprite_size.x / 2 * scale.x), position.y + (sprite_size.y / 2 * scale.y)}) ||
            edge_intersects(player->position, player->aftr.position, {position.x - (sprite_size.x / 2 * scale.x), position.y + (sprite_size.y / 2 * scale.y)}, {position.x - (sprite_size.x / 2 * scale.x), position.y - (sprite_size.y / 2 * scale.y)}));
}

Horde::Horde(Player* player):
        screen_center(window_width / 2, window_height / 2),
        time_elapsed(0),
        score(0),
        player(player),
        ghost_texture(ghost_sheet),
        heart_texture(animated_heart),
        hit_buffer(hit_path),
        hit_sound(hit_buffer),
        pickup_buffer(pick_path),
        pickup_sound(pickup_buffer){
            hit_sound.setVolume(volume);
            pickup_sound.setVolume(volume);
            srand(time(0));
}

//void Horde::update(float delta){}
bool Horde::update(float delta){
    update_horde(delta);
    update_hearts(delta);
    return spawn_enemies(delta);
}

void Horde::draw(sf::RenderWindow& window){
    for(Heart& h: hearts)
        h.draw(window);

    for(Ghost& g: horde)
        g.draw(window);
}

bool Horde::spawn_enemies(float delta){
    time_elapsed += delta;
    if(time_elapsed >= spawn_interval()){
        time_elapsed = 0;
        horde.emplace_back(screen_center + sf::Vector2f(700, sf::degrees(rand() % 360)), player, ghost_texture);
        return true;
    }
    return false;
}

bool Horde::spawn_hearts(sf::Vector2f position){
    if(rand() % 3 >= player->health){
        hearts.emplace_back(player, position, heart_texture);
        return true;
    }
    return false;
}

void Horde::update_horde(float delta){
    std::list<Ghost>::iterator g = horde.begin();
    while(g != horde.end()){
        if(g->update(delta)){
            hit_sound.play();
            score += (spawn_hearts(g->position)) ? 5 : 10;
            g = horde.erase(g);
        }
        else
            g++;
    }
}

void Horde::update_hearts(float delta){
    std::list<Heart>::iterator h = hearts.begin();
    while(h != hearts.end()){
        if(h->update(delta)){
            pickup_sound.play();
            h = hearts.erase(h);
        }
        else
            h++;
    }
}

unsigned Horde::spawn_interval(){
    if(score < 100)
        return 5;
    if(score < 300)
        return 4;
    if(score < 500)
        return 3;
    if(score < 1000)
        return 2;
    return 1;
}

void Horde::restart(Player* player){
    horde.clear();
    hearts.clear();
    time_elapsed = 0;
    score = 0;
    this->player = player;
}

State::State():
    player_texture(player_sheet),
    heart_texture(heart_sprite),
    backgournd_texture(background),
    heart(heart_texture),
    gameover_texture(gameover_path),
    gameover(gameover_texture),
    score_font(font_path),
    hit_buffer(player_hit_path),
    player(directions, player_texture, hit_buffer),
    horde(&player),
    score_file("score.txt", std::ios::in | std::ios::app),
    game_over(false),
    ost(ost_path),
    defeat_ost(defeat_path){
        heart.setScale(player_scale);
        gameover.setScale(player_scale);
        gameover.setOrigin(sf::Vector2f(200, 64));
        gameover.setPosition(sf::Vector2f(window_width / 2, window_height / 2));

        std::string line;
        if(std::getline(score_file, line))
            try{
                high_score = stoull(line);
            }
            catch(...){
                high_score = 0;
            }
        else
            high_score = 0;

        score_file.close();
        ost.setLooping(true);
        ost.setVolume(volume);
        ost.play();
        defeat_ost.setLooping(true);
        defeat_ost.setVolume(volume);
}

bool State::update(float delta){
    if(game_over)   return true;
    if(player.update(delta)){
        if(horde.score > high_score){
            high_score = horde.score;
            score_file.open("score.txt", std::ios::out | std::ios::trunc);
            score_file << high_score;
            score_file.flush();
            score_file.close();
        
        }
        ost.stop();
        defeat_ost.play();
        game_over = true;
        return true;
    }
    horde.update(delta);
    return false;
}

void State::draw(sf::RenderWindow& window){
    horde.draw(window);
    player.draw(window);
    if(!game_over){
        draw_background(window);
        draw_health(window);
        display_score(window);
    }
    else
        draw_gameover(window);
}

void State::draw_health(sf::RenderWindow& window){
    switch(player.health){
        case 3:
            heart.setPosition({155, 5});
            window.draw(heart);
        case 2:
            heart.setPosition({80, 5});
            window.draw(heart);
        case 1:
            heart.setPosition({5, 5});
            window.draw(heart);
    }
}

void State::display_score(sf::RenderWindow& window){
    sf::Text score_text(score_font, std::to_string(horde.score), 70);
    score_text.setFillColor(sf::Color::Black);
    score_text.setPosition(sf::Vector2f(window_width - 300, -10));
    window.draw(score_text);
}

void State::draw_background(sf::RenderWindow& window){
    sf::Vector2i size(32, 32);
    sf::Sprite tile(backgournd_texture);
    tile.setScale(sf::Vector2f(player_scale.x / 2, player_scale.y / 2));
    for(size_t j = 0; j < 9 ; j++){
        for(size_t i = 0; i < 16; i++){

            switch(j){
                case 0:
                    tile.setTextureRect(sf::IntRect(sf::Vector2i(32, 32), size));
                    break;
                case 1:
                    if(i == 0)
                        tile.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), size));
                    else if(i == 15)
                        tile.setTextureRect(sf::IntRect(sf::Vector2i(96, 0), size));
                    else
                        tile.setTextureRect(sf::IntRect(sf::Vector2i(32, 0), size));
                    break;
                case 8:
                    if(i == 0)
                        tile.setTextureRect(sf::IntRect(sf::Vector2i(0, 96), size));
                    else if(i == 15)
                        tile.setTextureRect(sf::IntRect(sf::Vector2i(96, 96), size));
                    else
                        tile.setTextureRect(sf::IntRect(sf::Vector2i(32, 96), size));
                    break;
                default:
                    if(i == 0)
                        tile.setTextureRect(sf::IntRect(sf::Vector2i(0, 32), size));
                    else if(i == 15)
                        tile.setTextureRect(sf::IntRect(sf::Vector2i(96, 32), size));
                    else
                        continue;
                    break;
            }

            tile.setPosition(sf::Vector2f(i * 80, j * 80));
            window.draw(tile);
        }
    }
}

void State::restart(){
    if(!game_over)  return;
    game_over = false;
    player = Player(directions, player_texture, hit_buffer);
    horde.restart(&player);
    defeat_ost.stop();
    ost.play();
}

void State::draw_gameover(sf::RenderWindow& window){
    window.draw(gameover);

    sf::Text retry(score_font, "Press [SPACE] to restart", 50);
    retry.setFillColor(sf::Color::Black);
    retry.setOutlineThickness(3);
    retry.setOutlineColor(sf::Color::White);
    retry.setPosition(sf::Vector2f(250, 600));
    window.draw(retry);

    retry.setPosition(sf::Vector2f(150, 75));
    retry.setString("score: " + std::to_string(horde.score) + "\nhigh score: " + std::to_string(high_score));
    window.draw(retry);
}

Heart::Heart(Player* player, sf::Vector2f position, sf::Texture& texture):
    position(position),
    sprite(texture),
    player(player),
    anim(animation_fps_period, h_sheet, heart_sprite_size),
    scale(player_scale.x){
        sprite.setOrigin(sf::Vector2f(heart_sprite_size.x / 2, heart_sprite_size.y / 2));
        sprite.setScale(player_scale);
        sprite.setPosition(position);
}

bool Heart::update(float delta){
    anim.update(delta);
    if(dist(position, player->position) < scale * 15){
        player->heal();
        return true;
    }
    return false;
}

void Heart::draw(sf::RenderWindow& window){
    sprite.setTextureRect(anim.get_sprite(0, false));
    window.draw(sprite);
}