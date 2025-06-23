#include "entities.hpp"
//#include "defaults.hpp"

void handle_close (sf::RenderWindow& window){
    window.close();
}

void handle_resize (const sf::Event::Resized& resized, sf::RenderWindow& window){
    /*sf::Vector2u new_size(100, 100);

    if(resized.size.x != window.getSize().x)
        new_size = sf::Vector2u(resized.size.x, resized.size.x * 9.f / 16.f);
    else if(resized.size.y != window.getSize().y)
        new_size = sf::Vector2u(resized.size.y * 16.f / 9.f, resized.size.y);
    window.setSize(new_size);*/

    window.setSize(sf::Vector2u(resized.size.x, resized.size.x * 9.f / 16.f));
}

void handle(const sf::Event::KeyPressed &KeyPressed, State &state){
    switch(KeyPressed.code){
        case sf::Keyboard::Key::W:
            state.directions[3] = true;
            break;
        case sf::Keyboard::Key::A:
            state.directions[1] = true;
            break;
        case sf::Keyboard::Key::S:
            state.directions[2] = true;
            break;
        case sf::Keyboard::Key::D:
            state.directions[0] = true;
            break;
    }

    if(KeyPressed.code == sf::Keyboard::Key::LShift)
        state.player.start_dash();
    if(KeyPressed.code == sf::Keyboard::Key::Space)
        state.restart();
}

void handle(const sf::Event::KeyReleased &KeyReleased, State &state){
    switch(KeyReleased.code){
        case sf::Keyboard::Key::W:
            state.directions[3] = false;
            break;
        case sf::Keyboard::Key::A:
            state.directions[1] = false;
            break;
        case sf::Keyboard::Key::S:
            state.directions[2] = false;
            break;
        case sf::Keyboard::Key::D:
            state.directions[0] = false;
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
    //sf::ContextSettings settings;
    //settings.antiAliasingLevel = 16;
    sf::RenderWindow window(sf::VideoMode ({1280, 720}), "Dasher");
    window.setVerticalSyncEnabled(true);
    //window.setFramerateLimit(1);

    State state;
    sf::Clock delta;
    sf::Color bg(sf::Color::Black);

    while (window.isOpen()){
        window.handleEvents([&window](const sf::Event::Closed&){handle_close(window);},
                            [&window](const sf::Event::Resized& event){handle_resize(event, window);},
                            [&state] (const auto& event){handle(event, state);});

        bg = (state.update(delta.restart().asSeconds()))? sf::Color::White: sf::Color::Black;

        window.clear(bg);
        state.draw(window);
        window.display();
    }
}