#include "Window.h"
#include "Header.h"
#include <SFML/Graphics.hpp>
#include <iostream>
Window::Window( int h , int w)
{
    Window::height = h ;
    Window::width = w ;
    Window::header = Header(h , w);
    sf::RenderWindow window(sf::VideoMode(Window::width, Window::height), "SFML window");

    getchar();
}
