#ifndef NEWSIZES_H
#define NEWSIZES_H
#include <SFML/Graphics.hpp>

class NewSizes
{
    public:
        NewSizes();
        virtual ~NewSizes();
        unsigned int menuBarHeight;
        unsigned int menuBarWidth;
        unsigned int footerBarHeight;
        unsigned int sidebarWidth;
        unsigned int sidebarHeight;
        unsigned int font;
        void UpdateSizes(sf::RenderWindow& window);
    protected:

    private:
};

#endif // NEWSIZES_H
