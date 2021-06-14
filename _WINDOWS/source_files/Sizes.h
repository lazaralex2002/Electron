#ifndef SIZES_H
#define SIZES_H

#include <SFML/Graphics.hpp>

struct {
    unsigned int windowWidth = 2480;
    unsigned int windowHeight = 1440;
    unsigned int menuBarHeight = 75;
    unsigned int footerBarHeight = 50;
    unsigned int sidebarWidth = 480;
    unsigned int font = 24;
    unsigned int menuBarWidth = window.getSize().x;
    unsigned int menuBarHeight = window.getSize().y * 1/10;
    unsigned int sidebarWidth = window.getSize().x * 4/20;
    unsigned int sidebarHeight = window.getSize().y * 8/10;
    unsigned int font = window.getSize().y * 24 / 500 ;
} Size;

#endif
