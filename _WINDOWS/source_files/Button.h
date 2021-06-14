#ifndef BUTTON_H
#define BUTTON_H

#include <SFML/Graphics.hpp>
#include "ResourcePath.hpp"

struct dimension {
    unsigned int width;
    unsigned int height;
};

struct point {
    unsigned int x;
    unsigned int y;
};

class Button
{
private:
    sf::RectangleShape shape;
    sf::Font* font;
    sf::Text text;
    sf::Color idleColor;
    sf::Color hoverColor;
    sf::Color activeColor;
    sf::Color textColor;
public:
    Button ();
    Button(point location,
           dimension s,
           std::string text,
           sf::Font* font,
           sf::Color idleColor,
           sf::Color hoverColor,
           sf::Color activeColor,
           sf::Color textColor);
    ~Button();

    // Functions
    void render(sf::RenderTarget* target);
};

#endif
