#include "NewSizes.h"
#include <SFML/Graphics.hpp>
NewSizes::NewSizes()
{
    //ctor
}

NewSizes::~NewSizes()
{
    //dtor
}

void NewSizes::UpdateSizes(sf::RenderWindow& window)
{
    NewSizes::menuBarWidth = window.getSize().x;
    NewSizes::menuBarHeight = window.getSize().y * 1/10;
    NewSizes::sidebarWidth = window.getSize().x * 4/20;
    NewSizes::sidebarHeight = window.getSize().y * 8/10;
    NewSizes::font = window.getSize().y * 24 / 500 ;
}
