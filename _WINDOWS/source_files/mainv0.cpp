#include <SFML/Graphics.hpp>
#include "ResourcePath.hpp"
#include "Button.h"
#include "Colors.h"

struct {
    unsigned int W, H, menuBarHeight, footerBarHeight, sidebarWidth, sidebarHeight, font;
} Size;

void updateSizes(sf::RenderWindow& target) {
    Size.W = sf::VideoMode::getDesktopMode().width * 3/4;
    Size.H = Size.W / 2;
    Size.menuBarHeight = 1/10 * Size.H;
    Size.footerBarHeight = 1/10 * Size.H;
    Size.sidebarWidth = 4/20 * Size.W;
    Size.font = 24;
}

using namespace std;

const unsigned W = sf::VideoMode::getDesktopMode().width * 3/4;
const unsigned H = W / 2;

void generateMenuBar(sf::RenderWindow& target, sf::Font* font, sf::RectangleShape& menuBar, Button& buttonNew, Button& buttonUndo, Button& buttonRedo);
void drawAndDisplayMenuBar(sf::RenderWindow& target, sf::RectangleShape& menuBar, Button& buttonNew, Button& buttonUndo, Button& buttonRedo );

void generateSidebar(sf::RenderWindow& target, sf::Font* font, sf::RectangleShape& sidebar);
void drawAndDisplaySidebar(sf::RenderWindow& target, sf::RectangleShape& sidebar);

int main()
{
    Size.W = sf::VideoMode::getDesktopMode().width * 3/4;
    Size.H = W / 2;
    sf::RenderWindow window(sf::VideoMode(W, H), "Electron", sf::Style::Default);
    updateSizes(window);
    sf::RectangleShape menuBar;
    Button buttonNew, buttonUndo, buttonRedo;
    sf::Font robotoRegular;
    robotoRegular.loadFromFile( + "Roboto-Regular.ttf") ;
    generateMenuBar(window, &robotoRegular, menuBar, buttonNew, buttonUndo, buttonRedo);
    drawAndDisplayMenuBar(window, menuBar, buttonNew, buttonUndo, buttonRedo);
    window.draw(menuBar);
    buttonNew.render(&window);
    buttonRedo.render(&window);
    buttonUndo.render(&window);
    
    // Sidebar
    sf::RectangleShape sidebar;
    generateSidebar(window, &robotoRegular, sidebar);
    drawAndDisplaySidebar(window, sidebar);
    
    
        window.display();
        while(window.isOpen())
        {
            sf::Event evnt;
            while(window.pollEvent(evnt))
            {
                switch(evnt.type)
                {
                    case sf::Event::Closed:
                        window.close();
                    case sf::Event::Resized:
                        window.clear();
                        if(window.getSize().x < 750 || window.getSize().y < 375)
                            window.setSize(sf::Vector2u(750, 375));
                        /*
                        if(window.getSize().x != window.getSize().y * 2)
                        {
                            if(window.getSize().x > window.getSize().y * 2) // width increased, increase height as well
                                window.setSize(sf::Vector2u(window.getSize().x, window.getSize().x*2));
                            else // height increased, increase width as well
                                window.setSize(sf::Vector2u(window.getSize().y*2, window.getSize().y));
                                
                        }
                        */
                        //generateMenuBar(window, &robotoRegular, menuBar, buttonNew, buttonUndo, buttonRedo);
                        //drawAndDisplayMenuBar(window, menuBar, buttonNew, buttonUndo, buttonRedo);
                    default:
                        break;
                }
            }
        }
        return 0 ;
}
void generateMenuBar(sf::RenderWindow& target, sf::Font* font, sf::RectangleShape& menuBar, Button& buttonNew, Button& buttonUndo, Button& buttonRedo)
{
    menuBar = sf::RectangleShape (sf::Vector2f(Size.W, Size.menuBarHeight));
    menuBar.setFillColor(grey[7]);
    buttonNew =  Button (point {12, 12},
                     dimension {100, 50},
                     "New",
                     font,
                     grey[5],
                     grey[6],
                     grey[9],
                     grey[0]);
    buttonUndo =  Button (point {124, 12},
                      dimension {100, 50},
                      "Undo",
                      font,
                      grey[5],
                      grey[6],
                      grey[9],
                      grey[0]);
    buttonRedo = Button (point {236, 12},
                      dimension {100, 50},
                      "Redo",
                      font,
                      grey[5],
                      grey[6],
                      grey[9],
                      grey[0]);
}

void drawAndDisplayMenuBar(sf::RenderWindow& target, sf::RectangleShape& menuBar, Button& buttonNew, Button& buttonUndo, Button& buttonRedo)
{
    target.draw(menuBar);
    buttonNew.render(&target);
    buttonRedo.render(&target);
    buttonUndo.render(&target);
    target.display();
}

void generateSidebar(sf::RenderWindow& target, sf::Font* font, sf::RectangleShape& sidebar)
{
    sidebar = sf::RectangleShape(sf::Vector2f(Size.sidebarWidth, Size.sidebarHeight));
    sidebar.setFillColor(grey[2]);
}

void drawAndDisplaySidebar(sf::RenderWindow& target, sf::RectangleShape& sidebar)
{
    target.draw(sidebar);
}

