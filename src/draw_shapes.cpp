////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <math.h>

////////////////////////////////////////////////////////////
/// Utility

template <typename T1, typename T2>
double dist(sf::Vector2<T1> p1, sf::Vector2<T2> p2)
{
    return sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}

const sf::Color GRAY(128, 128, 128, 255);

///
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
/// GUI state

struct Shape{
    unsigned type;
    unsigned color;
    float size = 30;
    sf::Vector2f position;

    Shape(unsigned t, unsigned c, sf::Vector2f p){
        type = t;
        color = c;
        position = p;
    }
};

struct State
{
    sf::RenderWindow window;
    const float bar_height = 50;
    const sf::Color colors[7] = {sf::Color::White, sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Cyan, sf::Color::Yellow, sf::Color::Magenta};
    unsigned current_shape = 0;
    unsigned current_color = 0;
    std::vector<Shape> shapes;
    int selected_shape = -1;
    sf::Vector2f drag_offset = {0.f, 0.f};

    State(unsigned w, unsigned h, std::string title)
    {
        window = sf::RenderWindow(sf::VideoMode({w, h}), title);
    }
};

///
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
/// Callback functions
void handle(const sf::Event::Closed &, State &gs)
{
    gs.window.close();
}

void handle(const sf::Event::TextEntered &textEnter, State &gs)
{
}

void handle(const sf::Event::KeyPressed &keyPressed, State &gs)
{
    if(keyPressed.code == sf::Keyboard::Key::Backspace && gs.selected_shape != -1){
        gs.shapes.erase(gs.shapes.begin() + gs.selected_shape);
        gs.selected_shape = -1;
    }
}

void handle(const sf::Event::MouseMoved &mouseMoved, State &gs)
{
    if(gs.selected_shape != -1)
        gs.shapes.at(gs.selected_shape).position = static_cast<sf::Vector2f>(mouseMoved.position) - gs.drag_offset;
}

void handle(const sf::Event::MouseButtonPressed &mouseBP, State &gs)
{
    if(mouseBP.position.y >= 0 && mouseBP.position.y <= gs.bar_height){
        const float color_bar_beginning = gs.window.getSize().x - gs.bar_height * 7;
        if(mouseBP.position.x >= 0 && mouseBP.position.x <= gs.bar_height * 5)
            gs.current_shape = mouseBP.position.x / 50;
        else if((mouseBP.position.x >= color_bar_beginning) && (mouseBP.position.x <= gs.window.getSize().x))
            gs.current_color = (mouseBP.position.x - color_bar_beginning) / 50;
    }
    else{
        if(mouseBP.button == sf::Mouse::Button::Left)
            gs.shapes.push_back(Shape(gs.current_shape, gs.current_color, static_cast<sf::Vector2f>(mouseBP.position)));
        if(mouseBP.button == sf::Mouse::Button::Right){
            for(size_t i = 0; i < gs.shapes.size(); i++){
                if(dist(gs.shapes.at(i).position, mouseBP.position) < gs.shapes.at(i).size){
                    gs.selected_shape = i;
                    gs.drag_offset = static_cast<sf::Vector2f>(mouseBP.position) - gs.shapes.at(i).position;
                    break;
                }
            }
        }
    }
}

void handle(const sf::Event::MouseButtonReleased &, State &gs)
{
    gs.selected_shape = -1;
}

void handle(const sf::Event::Resized &resized, State &gs)
{
    sf::FloatRect visibleArea({0.f, 0.f}, sf::Vector2f(resized.size));
    gs.window.setView(sf::View(visibleArea));
}

template <typename T>
void handle(const T &, State &gs)
{
    // All unhandled events will end up here
}
///
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
/// Graphics
void doGUI(State &gs)
{
    // TODO: here code to display the menus
    sf::RectangleShape button({gs.bar_height, gs.bar_height});
    button.setFillColor(sf::Color::Transparent);
    button.setOutlineThickness(-3.f);

    const float shape_position = gs.bar_height / 2;
    const float shape_radius = shape_position - 5;
    sf::CircleShape shape(shape_radius);
    shape.setOrigin({shape_radius, shape_radius});

    for(size_t i = 0; i < 5; i++){
        float position = gs.bar_height * i;
        button.setPosition({position, 0.f});
        button.setOutlineColor((i == gs.current_shape)? sf::Color::White: GRAY);
        gs.window.draw(button);
        shape.setPosition({position + shape_position, shape_position});
        shape.setPointCount((i == 0)? 30: i+2);
        gs.window.draw(shape);
    }

    for(size_t i = 0; i < 7; i++){
        button.setFillColor(gs.colors[6 - i]);
        button.setOutlineColor((6 - i == gs.current_color)? sf::Color::White: GRAY);
        float position = gs.window.getSize().x - (gs.bar_height * (i + 1));
        button.setPosition({position, 0.f});
        gs.window.draw(button);
    }
}

void doGraphics(State &gs)
{
    gs.window.clear();
    doGUI(gs);

    for(size_t i = 0; i < gs.shapes.size(); i++){
        sf::CircleShape tmp(gs.shapes.at(i).size, (gs.shapes.at(i).type == 0)? 30: gs.shapes.at(i).type + 2);
        tmp.setFillColor(gs.colors[gs.shapes.at(i).color]);
        if(i == gs.selected_shape){
            tmp.setOutlineColor(GRAY);
            tmp.setOutlineThickness(-3.f);
        }
        tmp.setOrigin({gs.shapes.at(i).size, gs.shapes.at(i).size});
        tmp.setPosition(gs.shapes.at(i).position);
        gs.window.draw(tmp);
    }

    // TODO: add here code to display shapes in your canvas

    gs.window.display();
}
///
////////////////////////////////////////////////////////////

int main()
{
    State gs(800, 600, "Draw shapes");
    //gs.window.setFramerateLimit(50);
    gs.window.setVerticalSyncEnabled(true);

    while (gs.window.isOpen()) // main loop
    {
        // event loop and handler through callbacks
        gs.window.handleEvents([&](const auto &event)
                               { handle(event, gs); });
        // Show update
        doGraphics(gs);
    }
}
