#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string.h>
#include <string>
#include <SFML/Graphics.hpp>
#include "ResourcePath.hpp"
#include "Colors.h"
#define MATRIX_W 40
#define MATRIX_H MATRIX_W / 2
#define MAX_COMPONENTE 100
#define INPUT_PATH "data.txt"
#define OUTPUT_PATH "data.txt"

FILE * input ;
FILE * output ;
int nrOfDrawedComponents;


struct {
    unsigned int W, H, menuBarW, menuBarH, footerBarW, footerBarH, sidebarW, sidebarH, canvasW, canvasH, font, wireH = 2;
} size1;


sf::Font robotoRegular;


int currentState = 0;

/*
 0 - selecting start dot
 1 - selecting end dot
 c
 */


struct dimension {
    int w;
    int h;
};


bool isEndHere [MATRIX_W][MATRIX_H];


struct coord {
    int x;
    int y;
};


struct point {
    int id; // electronical component identifier
    coord end; // coordinates of the place where the component is drawn towards
    bool invisible;
    char text[50];
};


bool move_state , text_state , delete_state;


struct create_occupied
{
    bool populated;
    coord start;
    int comp_nr;
    int k;
} occupied[MATRIX_W+1][MATRIX_H+1];


point matrix[MATRIX_W+1][MATRIX_H+1][4];
int filled[MATRIX_W+1][MATRIX_H+1][4];



struct {
    int status;
    int compId;
    int lastK;
    int lastKHover;
    coord start;
    coord lastHoveredPoint;
    bool cantPickStartHere;
} state;


struct floatCoord {
    float x;
    float y;
};


sf::Texture normalCursorTexture,hoverCursorTexture, moveCursorTexture, thrashBinCursorTexture , thrashBinCursorTransperentTexture,donTexture
          , normalCursorTransperentTexture, moveCursorTransperentTexture ,loadButtonTexture , saveButtonTexture , textButtonTexture , textButtonTransperentTexture;


struct {
    int minW;
    int minH;
    char nume[50];
    floatCoord rectangles[10][2];
    int rectanglesCount;
} compInfo[MAX_COMPONENTE];



enum button_states{ BTN_IDLE = 0 , BTN_HOVER , BTN_PRESSED} ;
bool anyShadowButton;



floatCoord floatMatrixToPx(floatCoord positionInMatrix)
{
    // float squareW = size1.canvasW / MATRIX_W;
    // float squareH = size1.canvasH / MATRIX_H;
    floatCoord temp;
    temp.x = positionInMatrix.x * ((float) 1 / (MATRIX_W+1) * size1.canvasW);
    temp.x += size1.sidebarW; // add offset
    temp.y = positionInMatrix.y * ((float) 1 / (MATRIX_H+1) * size1.canvasH);
    temp.y += size1.menuBarH; // add offset
    return temp;
}


//directions for crossing the matrix
//0 stands for going up, 1 for going left , 2 for right, 3 for down
int dx[] = { 0 , -1 , 1 , 0};
int dy[] = {-1 ,  0 , 0 , 1};

/*
 each point has an id which identifies the type of the electronical element that starts from it
 0 - empty
 1 - wire
 2 - battery
 */

/*
struct linkedList {
    char id[100];
    char direction[10];
    coord end;
    linkedList* next;
};
*/


sf::RectangleShape auxDemo;



enum Cursor_states { NORMAL_CURSOR = 0 , MOVE_CURSOR , TEXT_CURSOR , DELETE_CURSOR};



int Curent_Cursor_state;


const int nr_of_components = 11;
int start_of_componente_sidebar;
bool render_upButton;
bool render_downButton;
sf::RectangleShape shadowButton;
sf::Texture components_texture[nr_of_components];
sf::Texture upButton_texture ;
sf::Texture DownButton_texture ;


class Button
{
private:
    sf::Font* font;
    sf::Text text;
    sf::Color idleColor;
    sf::Color hoverColor;
    sf::Color activeColor;
    sf::Color textColor;
public:
    short unsigned buttonState;
    sf::RectangleShape shape;
    bool shadowButtonState;
    Button();
    Button(coord location,
           dimension s,
           std::string text,
           sf::Font* font,
           sf::Color idleColor,
           sf::Color hoverColor,
           sf::Color activeColor,
           sf::Color textColor);
    //Accessors
    bool isPressed();
    // Functions
    void update(sf::Vector2f mousePos );
    void render(sf::RenderTarget& target);
};


Button::Button()
{

}


Button::Button(coord location,
               dimension s,
               std::string text,
               sf::Font* font,
               sf::Color idleColor,
               sf::Color hoverColor,
               sf::Color activeColor,
               sf::Color textColor)
{
    this->shadowButtonState = 0;
    this->buttonState = BTN_IDLE;
    this->font = font;
    // Rectangle
    this->shape.setPosition(sf::Vector2f(location.x, location.y));
    this->shape.setSize(sf::Vector2f(s.w, s.h));
    // Content
    this->text.setFont(*this->font);
    this->text.setString(text);
    this->text.setFillColor(textColor);
    this->text.setCharacterSize(24);
    this->text.setPosition(
                           location.x + s.w / 2 - this->text.getGlobalBounds().width / 2,
                           location.y + s.h / 2 - this->text.getGlobalBounds().height
                           );
    // Style
    this->idleColor = idleColor;
    this->hoverColor = hoverColor;
    this->activeColor = activeColor;
    this->shape.setFillColor(this->idleColor);
}


void Button::render(sf::RenderTarget& target)
{
    target.draw(this->shape);
    target.draw(this->text);
    // buttonsList.insert(this, coordonates)
}


Button DorNButton;
Button upButton;
Button DownButton;
Button Componente_sidebar[4];
Button normalButtonCursor;
Button moveButtonCursor;
Button deleteButtonCursor;
Button saveButton;
Button loadButton;
Button textButton;


bool aux ;



void Button::update( sf::Vector2f mousePos )
{
    this->buttonState = BTN_IDLE;
    if ( this -> shape.getPosition().x < mousePos.x && this->shape.getPosition().x + this->shape.getSize().x > mousePos.x
        && this-> shape.getPosition().y < mousePos.y && this->shape.getPosition().y + this -> shape.getSize().y > mousePos.y )
    {
        this->buttonState = BTN_HOVER;
        if ( sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            this->buttonState = BTN_PRESSED;
        }
    }
    switch (this->buttonState)
    {
    case BTN_IDLE:
        this->shape.setFillColor(this->idleColor);
        break;
    case BTN_HOVER:
        this->shape.setFillColor(this->hoverColor);
        break;
    case BTN_PRESSED:
        this->shape.setFillColor(this->activeColor);
        aux = this->shadowButtonState;
        for ( int i = 0 ; i < 4 ; ++i )
        {
            Componente_sidebar[i].shadowButtonState = 0 ;
        }
        this->shadowButtonState = !aux;
        break;
    default:
        break;
    }
}



bool Button::isPressed()
{
    if ( this->buttonState==BTN_PRESSED) return true;
    return false;
}





void switchTheme()
{
    for(int i = 0; i < 5; i++)
    {
        sf::Color tmp = grey[i];
        grey[i].r = grey[9-i].r;
        grey[i].g = grey[9-i].g;
        grey[i].b = grey[9-i].b;
        grey[9-i].r = tmp.r;
        grey[9-i].g = tmp.g;
        grey[9-i].b = tmp.b;
        tmp = blue[i];
        blue[i].r = blue[9-i].r;
        blue[i].g = blue[9-i].g;
        blue[i].b = blue[9-i].b;
        blue[9-i].r = tmp.r;
        blue[9-i].g = tmp.g;
        blue[9-i].b = tmp.b;
    }
}




void loadTextures ()
{
    if (!hoverCursorTexture.loadFromFile(+"Files/hoverCursor.png"))std::cout<<"NU";
    if (!normalCursorTexture.loadFromFile(+"Files/normalCursor.jpg"))std::cout<<"NU";
    if (!normalCursorTransperentTexture.loadFromFile(+"Files/normalCursor.png"))std::cout<<"NU";
    if (!moveCursorTexture.loadFromFile(+"Files/moveCursor.png"))std::cout<<"NU";
    if (!moveCursorTransperentTexture.loadFromFile(+"Files/moveTransperentCursor.png"))std::cout<<"NU";
    if (!thrashBinCursorTexture.loadFromFile(+"Files/garbageBinCursor.png"))std::cout<<"NU";
    if (!thrashBinCursorTransperentTexture.loadFromFile(+"Files/garbageBinTransperentCursor.png"))std::cout<<"NU";
    if (!donTexture.loadFromFile(+"Files/donIcon.png"))std::cout<<"NU";
    if (!loadButtonTexture.loadFromFile(+"Files/loadIconTexture.png"))std::cout<<"NU";
    if (!saveButtonTexture.loadFromFile(+"Files/saveIconTexture.jpg"))std::cout<<"NU";
    if (!textButtonTexture.loadFromFile(+"Files/textCursorTexture.png"))std::cout<<"NU";
    if (!textButtonTransperentTexture.loadFromFile(+"Files/textCursorTextureTransperent.png"))std::cout<<"NU";
}



void render(sf::RenderWindow& target);




std::string to_string ( int i )
{
    std::string s ;
    int aux = i ;
    int p = 1 ;
    while ( aux )
    {
        aux/= 10 ;
        p *= 10 ;
    }
    p/=10;
    while (p)
    {
        s.push_back('0' + i /  p ) ;
        i = i % p ;
        p /= 10 ;

    }
    return s;
}





std::string to_string ( char s[] , int n )
{
    std::string str ;
    for ( int i = 0 ; i < n  ; ++i  )
    {
        str.push_back(s[i] ) ;
    }
    return str;
}





// Convert matrix coordinates into pixels coordinates
coord matrixToPx(coord positionInMatrix)
{
    // float squareW = size1.canvasW / MATRIX_W;
    // float squareH = size1.canvasH / MATRIX_H;
    coord temp;
    temp.x = positionInMatrix.x * ((float) 1 / (MATRIX_W+1) * size1.canvasW);
    temp.x += size1.sidebarW; // add offset
    temp.y = positionInMatrix.y * ((float) 1 / (MATRIX_H+1) * size1.canvasH);
    temp.y += size1.menuBarH; // add offset
    return temp;
}





// Convert location in pixels into the coordinates of the closest point relative to the matrix
coord pxToMatrix(coord a)
{
    coord positionInMatrix;
    a.x -= size1.sidebarW;
    a.y -= size1.menuBarH;
    /*
    canvas width .......... mouseX
    matrix width .......... x
     x = matrix width * mouseX / canvas width */
    // regula de 2 simpla somehow
    positionInMatrix.x = MATRIX_W * a.x / size1.canvasW + 1;
    positionInMatrix.y = MATRIX_H * a.y / size1.canvasH + 1;
    return positionInMatrix;
}





void updateSizes(sf::RenderWindow& target)
{
    size1.W = target.getSize().x;
    size1.H = target.getSize().y; // The height of the window is half of the height, aspect ratio 2:1
    size1.menuBarH = size1.footerBarH = (int)( (float)size1.H * 10/100.0); // each is 10% of the window's height
    size1.menuBarW = size1.footerBarW = size1.W; // 100% of the window's width
    size1.canvasH = (int)( (float)size1.H * 80/100.0 ); // fill the space between menu and footer (80% of window's height)
    size1.canvasW = (int)((float)size1.W * 8.0/10.0 ); // keep the 2:1 aspect ratio // canvasW is 80% of the window's width
    size1.sidebarH = size1.canvasH; // 85% of the window's height
    size1.sidebarW = size1.W - size1.canvasW; // fill the window's with with the canvas
}




void generateBackground(sf::RenderWindow& target)
{
    sf::RectangleShape background; // Initialize a rectangle to serve as background
    background = sf::RectangleShape(sf::Vector2f(size1.W, size1.H)); // It will have the window's size
    background.setFillColor(grey[1]); // Set the background color
    target.draw(background); // Draw the rectangle into the target
}





void generateMenuBar(sf::RenderWindow& target)
{
    sf::RectangleShape menuBar; // Initialize a rectangle to server as the menu bar's background
    menuBar = sf::RectangleShape(sf::Vector2f(size1.menuBarW, size1.menuBarH)); // Set its sizes
    menuBar.setFillColor(grey[3]); // Set its background
    menuBar.setPosition(0, 0); // Set the top right corner's position
    target.draw(menuBar); // Draw the background
}






void generateFooterBar(sf::RenderWindow& target)
{
    sf::RectangleShape footerBar; // Initialize a rectangle to server as the footer bar's background
    footerBar = sf::RectangleShape(sf::Vector2f(size1.footerBarW, size1.footerBarH)); // Set its sizes
    footerBar.setFillColor(grey[3]); // Set its background
    footerBar.setPosition(0, size1.H - size1.footerBarH); // Set the top right corner's position
    target.draw(footerBar);


    normalButtonCursor = Button (coord{ (int)(2.5 * size1.footerBarW / 100.0  ), static_cast<int>(size1.H - size1.footerBarH + (int)(10*size1.footerBarH/100.0)) } ,
                     dimension{ (int)(5.0 * size1.footerBarW / 100.0 ), (int)(float)(size1.footerBarH * 80.0 / 100.0) } ,
                     to_string("" , 0) , &robotoRegular ,
                     grey[4] , grey[5] , grey[6] , grey[7] );
    normalButtonCursor.update(sf::Vector2f(sf::Mouse::getPosition(target).x , sf::Mouse::getPosition(target).y ));
    normalButtonCursor.shape.setTexture( &normalCursorTexture ,  0);
    normalButtonCursor.render(target);





    //there is 1% of the footer bar's width spacing between the buttons
    moveButtonCursor = Button (coord{ (int)( 8.5  * size1.footerBarW / 100.0  ), static_cast<int>(size1.H - size1.footerBarH + (int)(10*size1.footerBarH/100.0)) } ,
                     dimension{ (int)(5.0 * size1.footerBarW / 100.0 ), (int)(float)(size1.footerBarH * 80.0 / 100.0) } ,
                     to_string("" , 0) , &robotoRegular ,
                     grey[4] , grey[5] , grey[6] , grey[7] );
    moveButtonCursor.update(sf::Vector2f(sf::Mouse::getPosition(target).x , sf::Mouse::getPosition(target).y ));
    moveButtonCursor.shape.setTexture( &moveCursorTexture ,  0);
    moveButtonCursor.render(target);

    textButton = Button (coord{ (int)( 14.5  * size1.footerBarW / 100.0  ), static_cast<int>(size1.H - size1.footerBarH + (int)(10*size1.footerBarH/100.0)) } ,
                     dimension{ (int)(5.0 * size1.footerBarW / 100.0 ), (int)(float)(size1.footerBarH * 80.0 / 100.0) } ,
                     to_string("" , 0) , &robotoRegular ,
                     grey[4] , grey[5] , grey[6] , grey[7] );
    textButton.update(sf::Vector2f(sf::Mouse::getPosition(target).x , sf::Mouse::getPosition(target).y ));
    textButton.shape.setTexture(&textButtonTexture , 1);
    textButton.render(target);


    deleteButtonCursor = Button (coord{ (int)( 20.5  * size1.footerBarW / 100.0  ), static_cast<int>(size1.H - size1.footerBarH + (int)(10*size1.footerBarH/100.0)) } ,
                     dimension{ (int)(5.0 * size1.footerBarW / 100.0 ), (int)(float)(size1.footerBarH * 80.0 / 100.0) } ,
                     to_string("" , 0) , &robotoRegular ,
                     grey[4] , grey[5] , grey[6] , grey[7] );
    deleteButtonCursor.update(sf::Vector2f(sf::Mouse::getPosition(target).x , sf::Mouse::getPosition(target).y ));
    deleteButtonCursor.shape.setTexture(&thrashBinCursorTexture , 1);
    deleteButtonCursor.render(target);


    saveButton = Button (coord{ (int)( 80.5  * size1.footerBarW / 100.0  ), static_cast<int>(size1.H - size1.footerBarH + (int)(10*size1.footerBarH/100.0)) } ,
                     dimension{ (int)(5.0 * size1.footerBarW / 100.0 ), (int)(float)(size1.footerBarH * 80.0 / 100.0) } ,
                     to_string("" , 0) , &robotoRegular ,
                     grey[4] , grey[5] , grey[6] , grey[7] );
    saveButton.update(sf::Vector2f(sf::Mouse::getPosition(target).x , sf::Mouse::getPosition(target).y ));
    saveButton.shape.setTexture( &saveButtonTexture , 1);
    saveButton.render(target);

    loadButton = Button (coord{ (int)( 86.5  * size1.footerBarW / 100.0  ), static_cast<int>(size1.H - size1.footerBarH + (int)(10*size1.footerBarH/100.0)) } ,
                     dimension{ (int)(5.0 * size1.footerBarW / 100.0 ), (int)(float)(size1.footerBarH * 80.0 / 100.0) } ,
                     to_string("" , 0) , &robotoRegular ,
                     grey[4] , grey[5] , grey[6] , grey[7] );
    loadButton.shape.setTexture( &loadButtonTexture , 1);
    loadButton.update(sf::Vector2f(sf::Mouse::getPosition(target).x , sf::Mouse::getPosition(target).y ));
    loadButton.render(target);

    /*deleteButtonCursor = Button (coord{ (int)( 14.5  * size1.footerBarW / 100.0  ), size1.H - size1.footerBarH + (int)(10*size1.footerBarH/100.0) } ,
                     dimension{ (int)(5.0 * size1.footerBarW / 100.0 ), (int)(float)(size1.footerBarH * 80.0 / 100.0) } ,
                     to_string("" , 0) , &robotoRegular ,
                     grey[4] , grey[5] , grey[6] , grey[7] );
    deleteButtonCursor.update(sf::Vector2f(sf::Mouse::getPosition(target).x , sf::Mouse::getPosition(target).y ));
    deleteButtonCursor.shape.setTexture( &thrashBinCursorTexture ,  1);
    deleteButtonCursor.render(target);

    deleteButtonCursor.update(sf::Vector2f(sf::Mouse::getPosition(target).x , sf::Mouse::getPosition(target).y ));
    deleteButtonCursor.render(target);
    */
    if ( !move_state && !text_state && !delete_state )
    {
        DorNButton = Button (coord{ (int)( 92.5 * size1.footerBarW / 100.0  ), static_cast<int>(size1.H - size1.footerBarH + (int)(10*size1.footerBarH/100.0)) } ,
                     dimension{ (int)(5.0 * size1.footerBarW / 100.0 ), (int)(float)(size1.footerBarH * 80.0 / 100.0) } ,
                     to_string("" , 0) , &robotoRegular ,
                     grey[4] , grey[5] , grey[6] , grey[7] );

        DorNButton.update(sf::Vector2f(sf::Mouse::getPosition(target).x , sf::Mouse::getPosition(target).y ));
        DorNButton.shape.setTexture(&donTexture , 1);
        DorNButton.render(target);
    }

}






void generateCanvas(sf::RenderWindow& target)
{
    sf::RectangleShape canvas; // Initialize a rectangle to server as the canvas's background
    canvas = sf::RectangleShape(sf::Vector2f(size1.canvasW, size1.canvasH)); // Set its sizes
    canvas.setFillColor(grey[0]); // Set its background
    canvas.setPosition(size1.sidebarW, size1.menuBarH); // Set the top right corner's position
    target.draw(canvas); // Draw the background
    // Draw the matrix
    // Draw the horizontal lines, one by one
    for(int i = 1; i <= MATRIX_H; i++) {
        float topOffset = size1.menuBarH + (i) * ((float) 1 / (MATRIX_H+1) * size1.canvasH);// its offset from top
        sf::RectangleShape line; // Initialize the line as a rectangle
        line = sf::RectangleShape(sf::Vector2f(size1.canvasW, 1)); // Set its size, the height is 1px
        line.setFillColor(grey[3]); // Set its color
        line.setPosition(size1.sidebarW, topOffset); // Set its position
        target.draw(line); // Draw it
    }
    // Draw the vertical lines, one by one
    for(int i = 1; i <= MATRIX_W; i++) {
        // std::cout << i << std::endl;
        float leftOffset = size1.sidebarW + (i) * ((float) 1 / (MATRIX_W+1) * size1.canvasW);// its offset from left
        sf::RectangleShape line; // Initialize the line as a rectangle
        line = sf::RectangleShape(sf::Vector2f(1, size1.canvasH)); // Set its size, the width is 1px
        line.setFillColor(grey[3]); // Set its color
        line.setPosition(leftOffset, size1.menuBarH); // Set its position
        target.draw(line); // Draw it
    }
    // draw indexes
    float squareW = size1.canvasW / MATRIX_W;
    for(int i = 1; i <= MATRIX_H; i++) {
        float topOffset = size1.menuBarH + (i) * ((float) 1 / (MATRIX_H+1) * size1.canvasH);// its offset from top

        std::string tmp ;
        tmp = to_string( i);
        char const *textContent = tmp.c_str();
        sf::Text text;
        text.setFont(robotoRegular);
        text.setString(textContent);
        text.setFillColor(grey[7]);
        text.setCharacterSize(15);
        text.setPosition(
                               size1.sidebarW - squareW / 1.5,
                               topOffset - text.getGlobalBounds().height
                               );

        text.setFont(robotoRegular);
        target.draw(text); // Draw it
    }
    for(int i = 1; i <= MATRIX_W; i++) {
        float leftOffset = size1.sidebarW + (i) * ((float) 1 / (MATRIX_W+1) * size1.canvasW);
        std::string tmp ;
        tmp = to_string(i);
        char const *textContent = tmp.c_str();
        sf::Text text;
        text.setFont(robotoRegular);
        text.setString(textContent);
        text.setFillColor(grey[7]);
        text.setCharacterSize(15);
        text.setPosition(
                               leftOffset - text.getGlobalBounds().width,
                                size1.menuBarH - squareW / 1.5
                               );

        text.setFont(robotoRegular);
        target.draw(text); // Draw it
    }
}





void drawLine(sf::RenderWindow& target, floatCoord start, floatCoord end)
{
    //printf("Drawing line from %f, %f to %f, %f...\n", start.x, start.y, end.x, end.y);
    // draw a line (not necessarely horizontal or vertical)
    floatCoord startPx = floatMatrixToPx(start);
    floatCoord endPx = floatMatrixToPx(end);
    if(startPx.x > endPx.x) {
        floatCoord temp = startPx;
        startPx = endPx;
        endPx = temp;
    }
    float length = sqrt( pow( startPx.x - endPx.x, 2 ) + pow( startPx.y - endPx.y, 2 ) );
    float m = (endPx.y - startPx.y) / (endPx.x - startPx.x); // slope / panta
    float angle = atan(m); // the angle between the points
    angle *= 180.0/3.141592653589793238463; // convert rad to deg
    // printf("%f - %f - deg\n", length, angle);
    sf::RectangleShape line;
    line = sf::RectangleShape(sf::Vector2f(length, size1.wireH));
    line.setFillColor(grey[7]);
    line.setPosition(startPx.x,
                     startPx.y );
    ++nrOfDrawedComponents;
    coord start1 = {(int)start.x , (int)start.y};
    coord end1 = { (int)end.x , (int)end.y};
    end1 = pxToMatrix( end1) ;
    start1 = pxToMatrix( start1 ) ;

    line.rotate(angle);
    target.draw(line);

    return;
}






void drawWire(sf::RenderWindow& target, int id, coord start, coord end, bool horizontal)
{
    coord startPx = matrixToPx(start);
    coord endPx = matrixToPx(end);
    if(horizontal)
    {
        float length = endPx.x - startPx.x + size1.wireH;
        sf::RectangleShape line;
        line = sf::RectangleShape(sf::Vector2f(length, size1.wireH));
        line.setFillColor(grey[7]);
        line.setPosition((float) startPx.x - (size1.wireH / 2),
                         (float) startPx.y - (size1.wireH / 2));
        target.draw(line);
    }
    else
    {
        float length = endPx.y - startPx.y + size1.wireH;
        sf::RectangleShape line;
        line = sf::RectangleShape(sf::Vector2f(size1.wireH, length));
        line.setFillColor(grey[7]);
        line.setPosition((float) startPx.x - (size1.wireH / 2),
                         (float) startPx.y - (size1.wireH / 2));
        target.draw(line);
    }
}






void sketch(sf::RenderWindow& target, int id, coord start, coord end, bool horizontal) {
    floatCoord rectangles[10][2];
    for(int i = 0; i < compInfo[id].rectanglesCount; i++)
        rectangles[i][0] = compInfo[id].rectangles[i][0],
        rectangles[i][1] = compInfo[id].rectangles[i][1];
    if(!horizontal)
        for(int i = 0; i < compInfo[id].rectanglesCount; i++)
        {
            std::swap(rectangles[i][0].x, rectangles[i][0].y);
            std::swap(rectangles[i][1].x, rectangles[i][1].y);
        }
    coord startPx = matrixToPx(start);
    for ( int i = 0; i < compInfo[id].rectanglesCount; i++) // for each line of the component
    {
        floatCoord lineStart = rectangles[i][0];
        lineStart.x -= 1;
        lineStart.y -= 1;
        floatCoord lineEnd = rectangles[i][1];
        lineEnd.x -= 1;
        lineEnd.y -= 1;
        floatCoord lineStartPx = floatMatrixToPx(lineStart);
        floatCoord lineEndPx = floatMatrixToPx(lineEnd);
        if(lineStart.y == lineEnd.y) { // horizontal line (start and end on the same line)
            float length = lineEndPx.x - lineStartPx.x + size1.wireH;
            sf::RectangleShape line;
            line = sf::RectangleShape(sf::Vector2f(length, size1.wireH));
            line.setFillColor(grey[7]);
            line.setPosition((float) startPx.x + lineStartPx.x - (size1.wireH / 2) - size1.sidebarW,
                             (float) startPx.y + lineStartPx.y - (size1.wireH / 2) - size1.menuBarH);
            target.draw(line);
        } else if (lineStart.x == lineEnd.x) { // vertical line (start and end on the same line)
            float length = (float) lineEndPx.y - lineStartPx.y + size1.wireH;
            sf::RectangleShape line;
            line = sf::RectangleShape(sf::Vector2f(size1.wireH, length));
            line.setFillColor(grey[7]);
            line.setPosition((float) startPx.x + lineStartPx.x - (size1.wireH / 2) - size1.sidebarW,
                             (float) startPx.y + lineStartPx.y - (size1.wireH / 2) - size1.menuBarH);
            target.draw(line);
        } else {
            drawLine(target, floatCoord { start.x + lineStart.x, start.y + lineStart.y }, floatCoord { start.x + lineEnd.x, start.y + lineEnd.y });
        }
    }

}




void drawElement(sf::RenderWindow& target, int id, coord start, coord end)
{
    if(id == 0)
        return;
    bool horizontal;
    if(start.y > end.y || start.x > end.x)
    {
        coord temp = start;
        start = end;
        end = temp;
    }
    if(start.y == end.y) // horizontal
        horizontal = true;
    else if(start.x == end.x)
        horizontal = false; // vertical
    else
        return; // something went wrong
    if(id == 1) {
        drawWire(target, id, start, end, horizontal);
    } else if(id < 100) {
        // if the length is too large, draw the element in the middle and add wires equally on both sides
        if(horizontal)
        {
            start.y -= 1;
            end.y -= 1;
            if(end.x - start.x + 1 > compInfo[id].minW)
            {
                int leftOffset, rightOffset;
                if((end.x - start.x + 1) % 2 == 1)
                {
                    leftOffset = (end.x - start.x + 1 - compInfo[id].minW) / 2;
                    rightOffset = (end.x - start.x + 1 - compInfo[id].minW) / 2 + 1;
                }
                else
                {
                    leftOffset = rightOffset = (end.x - start.x + 1 - compInfo[id].minW) / 2;
                }
                start.x += leftOffset;
                end.x -= rightOffset;
                if(leftOffset > 0)
                {
                    drawWire(target, 1, coord {start.x - leftOffset, start.y + 1}, coord{start.x, start.y + 1}, true);
                }
                if(rightOffset > 0)
                {
                    drawWire(target, 1, coord {end.x, end.y + 1}, coord {end.x + rightOffset, end.y + 1}, true);
                }
            }
        }
        else // vertical
        {
            start.x -= 1;
            end.x -= 1;
            if(end.y - start.y + 1 > compInfo[id].minW)
            {
                int topOffset, bottomOffset;
                if((end.y - start.y + 1) % 2 == 1)
                {
                    topOffset = (end.y - start.y + 1 - compInfo[id].minW) / 2;
                    bottomOffset = (end.y - start.y + 1 - compInfo[id].minW) / 2 + 1;
                }
                else
                {
                    topOffset = bottomOffset = (end.y - start.y + 1 - compInfo[id].minW) / 2;
                }
                start.y += topOffset;
                end.y -= bottomOffset;
                if(topOffset > 0)
                {
                    drawWire(target, 1, coord {start.x + 1, start.y - topOffset}, coord {start.x + 1, start.y}, false);
                }
                if(bottomOffset > 0)
                {
                    drawWire(target, 1, coord {start.x + 1, end.y}, coord {end.x + 1, end.y + bottomOffset}, false);
                }
            }
        }
        sketch(target, id, start, end, horizontal);
    }
}




void drawInfo(sf::RenderWindow& target, coord start, int k) {
    coord end = matrix[start.x][start.y][k].end;
    //printf("\nhey %d %d - %d %d\n", start.x, start.y, end.x, end.y);
    floatCoord textPosition, pxTextPosition;
    sf::Text text;
    text.setFont(robotoRegular);
    text.setString(matrix[start.x][start.y][k].text);
    text.setCharacterSize(18); // in pixels, not points!
    text.setFillColor(grey[7]);
    if(start.x == end.x) { // vertical
        textPosition.x = (float) start.x - 1.5;
        textPosition.y = (float) (start.y + end.y - 1) / 2;
        pxTextPosition = floatMatrixToPx(textPosition);
        text.setPosition(pxTextPosition.x, pxTextPosition.y);
        text.setOrigin(text.getLocalBounds().width / 2.0f, text.getLocalBounds().height / 2.0f);
        text.rotate(-90);
        target.draw(text);
    } else if(start.y == end.y) { // horizontal
        textPosition.x = (float) (start.x + end.x - 1) / 2;
        textPosition.y = (float) start.y - 1.5;
        pxTextPosition = floatMatrixToPx(textPosition);
        text.setPosition(pxTextPosition.x, pxTextPosition.y);
        text.setOrigin(text.getLocalBounds().width / 2.0f, text.getLocalBounds().height / 2.0f);
        target.draw(text);
    }
}




// Iterate through all the matrix and call drawElement function for each point whose id is not 0
// (which means it's defined / is not empty)
void drawElements(sf::RenderWindow& target)
{
    // Parse each dot
    // For each element in dot's "in" list, draw element
    for(int i = 1; i <= MATRIX_W; i++) // Each line
    {
        for(int j = 1; j <= MATRIX_H; j++) // Each column
        {
            for(int k = 0; k < 4; k++)
            {
                // If a electrical components starts here (has a defined id / != 0)
                if(matrix[i][j][k].id != 0) {
                    drawElement(target, matrix[i][j][k].id, coord{i, j}, matrix[i][j][k].end);
                    if(strlen(matrix[i][j][k].text) > 0)
                        drawInfo(target, {i, j}, k);
                }
            }
        }
    }
}





void defineComponents()
{
    FILE *f;
    f = fopen("components.txt", "r");
    if(f == NULL) {
        perror("Error opening file");
        exit(1);
    }
    while(!feof(f)) {
        int id;
        char title[50];
        int w, h, count;
        fscanf(f, "%d", &id);
        //printf("%d\n", id);
        fscanf(f, "%s", title);
        fscanf(f, "%d %d", &h, &w);
        fscanf(f, "%d", &count);
        strcpy(compInfo[id].nume, title);
        compInfo[id].minH = h;
        compInfo[id].minW = w;
        compInfo[id].rectanglesCount = count;
        float x1, y1, x2, y2;
        for(int i = 0; i < count; i++) {
            fscanf(f, "%f %f", &x1, &y1);
            fscanf(f, "%f %f", &x2, &y2);
            compInfo[id].rectangles[i][0] = {x1, y1};
            compInfo[id].rectangles[i][1] = {x2, y2};
        }
    }
}





coord closestPossibleEnd(coord start, coord current, int compId, int k)
{
    coord end;
    end.x = -1;
    end.y = -1;
    int w = compInfo[compId].minW;
    int h = compInfo[compId].minH;
    if(start.x == current.x && start.y == current.y) // previewing
    {
        if(start.x + w - 1 <= MATRIX_W) // try placing it towards right
        {
            if(!filled[start.x + w - 1][start.y][k])
            {
                end.x = start.x + w - 1;
                end.y = start.y;
                    return end;
            }
        } else { // try placing it towards left
            if(!filled[start.x - w + 1][start.y][k])
            {
                end.x = start.x - w + 3;
                end.y = start.y;
                    return end;
            }
        }
        if(start.x + h - 1 <= MATRIX_H) // try placing it downwards
        {
            if(!filled[start.x][start.y + h - 1][k])
            {
                end.x = start.x;
                end.y = start.y + h - 1;
                    return end;
            }
        } else { // try placing it upwards
            if(!filled[start.x][start.y - h + 1][k])
            {
                end.x = start.x;
                end.y = start.y - h + 1;
                    return end;
            }
        }
    } else { // not just previewing, but choosing the end of the element
        int dX, dY; // delta
        dX = abs(current.x - start.x) + 1;
        dY = abs(current.y - start.y) + 1;
        if(dX >= dY) // try inserting it horizontally
        {
            int missingX = 0;
            if(abs(current.x - start.x + 1) < w) {
                missingX = w - abs(current.x - start.x + 1);
            }
            end.y = start.y;
            end.x = current.x + (current.x > start.x ? missingX : (-1) * missingX);
            if ( current.x < start.x && abs(current.x - start.x )  < compInfo[state.compId].minW )
            {
                end.x += 2 ;
            }
            if(!filled[end.x][end.y][k])
            {
                    return end;
            }
        } else { // try insert it vertically
            int missingY = 0;
            if(abs(current.y - start.y + 1) < w) {
                missingY = w - abs(current.y - start.y + 1);
            }
            end.x = start.x;
            end.y = current.y + (current.y > start.y ? missingY : (-1) * missingY);
            if ( current.y < start.y && abs( current.y - start.y) < compInfo[state.compId].minW)
            {
                end.y += 2 ;
            }
            if(!filled[end.x][end.y][k])
            {
                    return end;
            }

        }
    }
    return end;
}





void drawCursor ( sf::RenderWindow& target  )
{
    sf::RectangleShape cursor ( sf::Vector2f ( size1.W / 100.0 * 4 , size1.H/100.0 * 6 ) );
    cursor.setPosition( sf::Mouse::getPosition(target).x , sf::Mouse::getPosition(target).y);
    switch(Curent_Cursor_state)
        {
        case NORMAL_CURSOR:
            cursor.setTexture(&normalCursorTransperentTexture , 1 );
            break;
        case MOVE_CURSOR:
            cursor.setTexture(&moveCursorTransperentTexture , 1);
            cursor.setPosition( sf::Mouse::getPosition(target).x - 27, sf::Mouse::getPosition(target).y - 20);
            break;
        case TEXT_CURSOR:
            cursor.setTexture(&textButtonTransperentTexture , 1);
            cursor.setPosition( sf::Mouse::getPosition(target).x - 27, sf::Mouse::getPosition(target).y - 20);
            break;
        case DELETE_CURSOR:
            cursor.setTexture(&thrashBinCursorTransperentTexture , 1 );
            cursor.setPosition( sf::Mouse::getPosition(target).x - 27, sf::Mouse::getPosition(target).y - 20);
        default:
            break;
        }
    target.draw(cursor);
}




void highlightDot(sf::RenderWindow& target, coord positionInMatrix)
{
    if ( Curent_Cursor_state == MOVE_CURSOR || Curent_Cursor_state == TEXT_CURSOR || Curent_Cursor_state == DELETE_CURSOR )
    {
        render(target);
        drawCursor(target);
        target.display();
    }
    else
    {
        // Convert the coordinates in pixels into coordinates of matrix
        target.clear(); // clear the window
        render(target); // draw all components
        coord positionInPixels = matrixToPx(positionInMatrix);
        // Draw the dot's white border
        sf::CircleShape dotBg(30); // initialize the bg circle
        dotBg.setFillColor(grey[1]); // bg's color
        dotBg.setPosition(positionInPixels.x-8, positionInPixels.y-8); // dot's bg position
        dotBg.setRadius(8); // bg radius
        dotBg.setPointCount(30); // some sort of border radius
        target.draw(dotBg); // draw the bg
        // Draw the dot itself
        sf::CircleShape dot(30); // initialize dot of radius ...
        dot.setFillColor(blue[7]); // dot's color
        dot.setPosition(positionInPixels.x-6, positionInPixels.y-6); // dot position
        dot.setRadius(6); // dot radius
        dot.setPointCount(30); // some sort of border radius
        target.draw(dot); // draw the dot
        target.display(); // display the window
    }
}




void mouseMoved(sf::RenderWindow& target)
{
    int x = sf::Mouse::getPosition(target).x,
        y = sf::Mouse::getPosition(target).y;

    // If the mouse is inside the canvas
    if(x >= size1.sidebarW && x <= size1.W && y >= size1.menuBarH && y <= size1.H - size1.footerBarH)
    { // Inside the canvas
        coord matrixCoord = pxToMatrix(coord{x,y});
        /*if(state.lastHoveredPoint.x == matrixCoord.x && state.lastHoveredPoint.y == matrixCoord.y)
            return;
        else {
            state.lastHoveredPoint.x = matrixCoord.x;
            state.lastHoveredPoint.y = matrixCoord.y;
        }
        */
        target.setMouseCursorVisible(0);
        if ( !move_state && !text_state && !delete_state )// temporary fix, the if can be removed
        {

        }
        switch(currentState) {
            // Selecting starting dot, highlight closest dot
            case 0:
                highlightDot(target, pxToMatrix(coord{x, y}));
                break;
        }
        switch(state.status) {
            case 0:
            {
                for(int i = 1; i <= MATRIX_W; i++) // Each line
                {
                    for(int j = 1; j <= MATRIX_H; j++) // Each column
                    {
                        for(int k = 0; k < 4; k++)
                        {
                            if(matrix[i][j][k].invisible) {
                                matrix[i][j][k].id = 0;
                            }
                        }

                    }
                }

                int k = 0;
                while(k < 4 && matrix[matrixCoord.x][matrixCoord.y][k].id != 0)
                    k += 1;
                if(k >= 4) {
                    state.cantPickStartHere = true;
                    return;
                } else {
                    state.cantPickStartHere = false;
                }



                /*
                if(matrix[matrixCoord.x][matrixCoord.y][k].id != 0)
                    break;
                 */



                coord closestEnd = closestPossibleEnd(matrixCoord, matrixCoord, state.compId, 0);
                if(closestEnd.x != -1 && closestEnd.y != -1)
                {
                    matrix[matrixCoord.x][matrixCoord.y][k] = point{state.compId,
                        closestEnd
                    };
                    state.lastKHover = k;
                    matrix[matrixCoord.x][matrixCoord.y][k].invisible = true;
                }
                /*
                 matrix[matrixCoord.x][matrixCoord.y] = point{state.compId, coord{matrixCoord.x+compInfo[state.compId].minW-1,matrixCoord.y}};
                matrix[matrixCoord.x][matrixCoord.y].invisible = true;
                 */
                break;
            }
            case 1:
            {
                coord matrixCoord = pxToMatrix(coord{x,y});
                coord closestEnd = closestPossibleEnd(state.start, matrixCoord, state.compId, 0);
                // printf("current: %d, %d - closest: %d, %d\n", matrixCoord.x, matrixCoord.y, closestEnd.x, closestEnd.y);
                if(closestEnd.x != -1 && closestEnd.y != -1)
                    {
                        if ( closestEnd.x >= 1 && closestEnd.x <= MATRIX_W && closestEnd.y >= 1 && closestEnd.y <= MATRIX_H )
                        {
                            matrix[state.start.x][state.start.y][state.lastK].end = closestEnd;
                        }
                        else
                        {
                            return;
                        }
                    }
                break;

            }
        }
    }
    else
    {
        for ( int i = 1 ; i <= MATRIX_W ; ++i )
        {
            for ( int j = 1 ; j <= MATRIX_H ; ++j )
            {
                for ( int k = 0 ; k < 4 ; ++k )
                    if ( matrix[i][j][k].invisible )
                    {
                        matrix[i][j][k].id = 0 ;
                    }
            }
        }
        drawCursor ( target );
        //target.setMouseCursorVisible(1);
        render( target);
        drawCursor(target);
        target.display();
    }
}





void changeCompInfo(sf::RenderWindow& target, coord start, int k) {
    if(matrix[start.x][start.y][k].id == 1)
        return;
    printf("Change this %s's info:\n", compInfo[ matrix[start.x][start.y][k].id ].nume);
    char text[50];
    // scanf("%s", text);
    std::cin.getline(text, 50);
    strcpy(matrix[start.x][start.y][k].text, text);
    printf("%s's info changed successfully.\n", compInfo[ matrix[start.x][start.y][k].id ].nume);
    render(target);
}




void mouseClicked(sf::RenderWindow& target)
{
    int x = sf::Mouse::getPosition(target).x,
        y = sf::Mouse::getPosition(target).y;
    // If the mouse is inside the canvas
    if(x >= size1.sidebarW && x <= size1.W && y >= size1.menuBarH && y <= size1.H - size1.footerBarH) { // Inside the canvas
        switch(state.status) {
            case 0:
            {
                if(text_state) {
                    coord matrixCoord = pxToMatrix(coord{x,y});
                    if(occupied[matrixCoord.x][matrixCoord.y].populated) {
                        changeCompInfo(target,
                                       occupied[matrixCoord.x][matrixCoord.y].start,
                                       occupied[matrixCoord.x][matrixCoord.y].k);
                        return;
                    } else {
                        return;
                    }
                }
                else
                {
                    if(state.cantPickStartHere == true)
                    return;
                    coord matrixCoord = pxToMatrix(coord{x,y});
                    matrix[matrixCoord.x][matrixCoord.y][state.lastKHover].invisible = false;
                    state.start = matrixCoord;
                    state.status = 1;
                    state.lastK = state.lastKHover;
                    isEndHere[matrixCoord.x][matrixCoord.y] = 1 ;
                    break;
                }
            }
            case 1:
            {
                coord matrixCoord = pxToMatrix(coord{x,y});
                coord closestEnd = closestPossibleEnd(state.start, matrixCoord, state.compId, 0);
                if ( closestEnd.x >= 1 && closestEnd.x <= MATRIX_W && closestEnd.y >= 1 && closestEnd.y <= MATRIX_H )
                {
                    //matrix[state.start.x][state.start.y][state.lastK].end = closestEnd;
                }
                else
                {
                    return;
                }
                if ( state.start.x == closestEnd.x )
                {
                    if ( state.compId == 1 )
                    {
                       int miny = state.start.y ;
                       int maxy = closestEnd.y ;
                       if ( maxy < miny ) std::swap( maxy , miny );
                       for ( int y = miny + 1 ; y < maxy  ; ++y )
                       {
                           if ( occupied[state.start.x][y].comp_nr > 0 )
                           {
                               coord ffs = occupied[state.start.x][y].start;
                               for ( int k = 0 ; k < 4 ; ++k )
                               {
                                   if ( matrix[ffs.x][ffs.y][k].id != 0 )
                                   {
                                        if ( ffs.x < state.start.x && matrix[ffs.x][ffs.y][k].end.x > state.start.x
                                           && matrix[ffs.x][ffs.y][k].id >= 1 ) return; // the wire met a horizontal component
                                        else
                                        {
                                            if ( matrix[ffs.x][ffs.y][k].end.x == state.start.x  ) return ;
                                        }
                                   }
                               }
                           }
                       }
                    }
                    else
                    {
                        coord mid = {state.start.x , (state.start.y + closestEnd.y)/2 };
                        for ( int y = mid.y - compInfo[state.compId].minW / 2 + 2; y <= mid.y + compInfo[state.compId].minW / 2 - 1; ++y )//to be tested if it goes outside the bounds
                        {
                            if ( occupied[state.start.x][y].comp_nr > 0 || occupied[state.start.x - 1][y].comp_nr > 0 || occupied[state.start.x + 1][y].comp_nr > 0)
                                return ;
                        }
                    }
                }
                else
                {
                    if ( state.compId == 1 )
                    {
                         if ( occupied[x][state.start.y].comp_nr > 0 )
                           {
                               coord ffs = occupied[y][state.start.x].start;
                               for ( int k = 0 ; k < 4 ; ++k )
                               {
                                   if ( matrix[ffs.x][ffs.y][k].end.x == ffs.x )
                                        if ( matrix[ffs.x][ffs.y][k].id > 1 )
                                            return ;
                               }
                           }
                    }
                    else
                    {
                        coord mid = {(state.start.x + closestEnd.x )/ 2 , state.start.y};
                        for ( int x = mid.x - compInfo[state.compId].minW / 2 + 2 ; x <= mid.x + compInfo[state.compId].minW / 2 -1; ++x )
                        {
                            if ( occupied[x][state.start.y -1].comp_nr >= 1 || occupied[x][state.start.y +1].comp_nr >= 1 || occupied[x][state.start.y].comp_nr >= 1)
                                return;
                        }

                    }
                }
                if ( state.compId >= 1 )
                {
                    isEndHere[closestEnd.x][closestEnd.y] = 1 ;
                    matrix[state.start.x][state.start.y][state.lastK].end = closestEnd;
                    state.status = 0;
                    ++nrOfDrawedComponents ;
                    if ( state.start.x == closestEnd.x )//vertical
                    {
                            if ( state.compId == 1 )
                            {
                                int temp1 = state.start.y ;
                                int temp2 = closestEnd.y ;
                                if ( temp1 > temp2 )
                                {
                                    std::swap ( temp1 , temp2 ) ;
                                }
                                for ( int y = temp1 ; y <= temp2 ; ++y )
                                {
                                    occupied[state.start.x][y] = { 1 , state.start , nrOfDrawedComponents , state.lastK};
                                }
                            }
                            else
                            {
                                coord mid = {state.start.x , (state.start.y + closestEnd.y)/2 };
                                for ( int y = mid.y - compInfo[state.compId].minW / 2 + 2; y <= mid.y + compInfo[state.compId].minW / 2 - 1; ++y )//to be tested if it goes outside the bounds
                                {
                                    occupied[state.start.x][y] = { 1 , state.start , nrOfDrawedComponents , state.lastK };
                                    occupied[state.start.x-1][y] = { 1 , state.start , nrOfDrawedComponents , state.lastK};
                                    occupied[state.start.x+1][y] = { 1 , state.start , nrOfDrawedComponents , state.lastK};
                                }
                            }


                    }
                    else
                    {
                        if ( state.start.y == closestEnd.y) //horizontal
                        {
                            if ( state.compId == 1 )
                            {
                                int temp1 = state.start.x ;
                                int temp2 =  closestEnd.x ;
                                if ( temp1 > temp2 )
                                {
                                    std::swap ( temp1 , temp2 ) ;
                                }
                                for ( int x = temp1 ; x <= temp2 ; ++x )
                                {
                                    occupied[x][state.start.y] = { 1 , state.start , nrOfDrawedComponents , state.lastK};
                                }
                            }
                            else
                            {
                                coord mid = {(state.start.x + closestEnd.x )/ 2 , state.start.y};
                                for ( int x = mid.x - compInfo[state.compId].minW / 2 + 2 ; x <= mid.x + compInfo[state.compId].minW / 2 -1; ++x )
                                {
                                    occupied[x][state.start.y -1] = {1 , state.start , nrOfDrawedComponents , state.lastK };
                                    occupied[x][state.start.y] =    {1 , state.start , nrOfDrawedComponents , state.lastK};
                                    occupied[x][state.start.y +1] = {1 , state.start , nrOfDrawedComponents , state.lastK };
                                }
                            }

                        }
                    }
                }
                break;
            }

        }
    }
}




void load_components ( )
{
    if (!components_texture[1].loadFromFile(+"Files/wire.png"))std::cout<<"NU";
    if (!components_texture[2].loadFromFile(+"Files/resistor.png"))std::cout<<"NU";
    if (!components_texture[3].loadFromFile(+"Files/cell.png"))std::cout<<"NU";
    if (!components_texture[4].loadFromFile(+"Files/battery.png"))std::cout<<"NU";
    if (!components_texture[5].loadFromFile(+"Files/rheostat.png"))std::cout<<"NU";
    if (!components_texture[6].loadFromFile(+"Files/potentiometer.png"))std::cout<<"NU";
    if (!components_texture[7].loadFromFile(+"Files/capacitor.png"))std::cout<<"NU";
    if (!components_texture[8].loadFromFile(+"Files/diode.png"))std::cout<<"NU";
    if (!components_texture[9].loadFromFile(+"Files/vericap.png"))std::cout<<"NU";
    if (!components_texture[10].loadFromFile(+"Files/switch.png"))std::cout<<"NU";
    if (!upButton_texture.loadFromFile(+"Files/upArrow.png"))std::cout<<"NU";
    if (!DownButton_texture.loadFromFile(+"Files/downArrow.png"))std::cout<<"NU";
}




void download()
{
    printf("Saving...\n");
    output = fopen(OUTPUT_PATH, "w");
    if(!output) {
        printf("Error opening output file.");
        exit(1);
    }
    for(int i = 1; i <= MATRIX_W; i++) {
        for(int j = 1; j <= MATRIX_H; j++) {
            for(int k = 0; k < 4; k++) {
                if(matrix[i][j][k].id == 0)
                    continue;
                //fprintf(f, "%d, %d %d id %d end %d, %d, inv %d text %s\n",
                fprintf(output, "%d %d %d %d %d %d %d ",
                        i, j, k,
                        matrix[i][j][k].id,
                        matrix[i][j][k].end.x, matrix[i][j][k].end.y,
                        matrix[i][j][k].invisible ? 1:0);
                // strlen(matrix[i][j][k].text)>0?matrix[i][j][k].text:"~");
                bool ok = false;
                for(int temp = 0; temp < strlen(matrix[i][j][k].text); temp++ )
                    if(matrix[i][j][k].text[temp] != ' ' && matrix[i][j][k].text[temp != '~'])
                    {
                        ok = true;
                        break;
                    }
                if(ok)
                    fprintf(output, "%s\n", matrix[i][j][k].text);
                else
                    fprintf(output, "~\n");

            }
        }
    }
    printf("Data saved.\n");
}

void upload(sf::RenderWindow &target)
{
    for ( int i = 1 ; i <= MATRIX_W ; ++i )
    {
        for ( int j = 1 ; j <= MATRIX_W ; ++j )
        {
            for ( int k = 0 ; k < 4 ; ++k )
            {
                matrix[i][j][k].id = 0;
                matrix[i][j][k].invisible = 1 ;
                matrix[i][j][k].text[0] = NULL ;
                matrix[i][j][k].end = coord { 0 , 0};
            }
            occupied[i][j].populated = false;
        }
    }
    printf("Uploading...\n");
    input = fopen( INPUT_PATH , "r");
    if(!input) {
        printf("Error opening input file.");
        exit(1);
    }
    nrOfDrawedComponents = 0;
    while(!feof(input)) {
        int a, b, c, id, inv, endx, endy;
        char text[50];
        fscanf(input, "%d %d %d %d %d %d %d %s", &a, &b, &c, &id, &endx, &endy, &inv, text);
        bool invisible = inv == 1 ? true:false;
        if(text[0] == '~')
            text[0] = ' ';
        if(a >= 1 && a <= MATRIX_W && b >= 1 && b <= MATRIX_H
           && c >= 0 && c <= 3
           && endx >= 1 && endx <= MATRIX_W && endy >= 1 && endy <= MATRIX_H) {
            matrix[a][b][c] = point {
                id,
                coord {endx, endy},
                invisible
            };
            strcpy(matrix[a][b][c].text, text);
            if(id == 0)
                continue;
            nrOfDrawedComponents += 1;
            if ( a == endx)//vertical
            {
                    if ( id == 1 )
                    {
                        int temp1 = a ;
                        int temp2 = endx ;
                        if ( temp1 > temp2 )
                        {
                            std::swap ( temp1 , temp2 ) ;
                        }
                        for ( int y = temp1 ; y <= temp2 ; ++y )
                        {
                            occupied[a][b] = { 1 , {a,b} , nrOfDrawedComponents , c};
                        }
                    }
                    else
                    {
                        coord mid = {a , (b + endy)/2 };
                        for ( int y = mid.y - compInfo[id].minW / 2 + 2; y <= mid.y + compInfo[id].minW / 2 - 1; ++y )//to be tested if it goes outside the bounds
                        {
                            occupied[a][y] = { 1 , {a,b} , nrOfDrawedComponents , c };
                            occupied[a-1][y] = { 1 , {a,b} , nrOfDrawedComponents , c};
                            occupied[a+1][y] = { 1 , {a,b} , nrOfDrawedComponents , c};
                        }
                    }

            }
            else
            {
                if ( b == endy) //horizontal
                {
                    if ( id == 1 )
                    {
                        int temp1 = a ;
                        int temp2 =  endx ;
                        if ( temp1 > temp2 )
                        {
                            std::swap ( temp1 , temp2 ) ;
                        }
                        for ( int x = temp1 ; x <= temp2 ; ++x )
                        {
                            occupied[x][b] = { 1 , {a,b} , nrOfDrawedComponents , c};
                        }
                    }
                    else
                    {
                        coord mid = {(a + endx )/ 2 , b};
                        for ( int x = mid.x - compInfo[id].minW / 2 + 2 ; x <= mid.x + compInfo[id].minW / 2 -1; ++x )
                        {
                            occupied[x][b -1] = {1 , {a,b} , nrOfDrawedComponents , c };
                            occupied[x][b] =    {1 , {a,b} , nrOfDrawedComponents , c };
                            occupied[x][b +1] = {1 , {a,b} , nrOfDrawedComponents , c };
                        }
                    }

                }
            }
        }
    }
    render(target);
    printf("Data loaded.\n");
}

void generateSidebar(sf::RenderWindow& target)
{
    sf::RectangleShape sidebar; // Initialize a rectangle to server as the sidebar's background
    sidebar = sf::RectangleShape(sf::Vector2f(size1.sidebarW, size1.sidebarH)); // Set its sizes
    sidebar.setFillColor(grey[2]); // Set its background
    sidebar.setPosition(0, size1.menuBarH); // Set the top right corner's position
    target.draw(sidebar);
    if ( !move_state && !text_state && !delete_state )
    {
        upButton= Button (coord{ static_cast<int>(size1.sidebarW/3) , static_cast<int>(size1.menuBarH + (int)(2.5*size1.sidebarH / 100.0)) } ,
                      dimension{static_cast<int>(size1.sidebarW/3) , (int)(float)(size1.sidebarH * 10.0 / 100.0) } ,
                     to_string("" , 0) , &robotoRegular ,
                     grey[4] , grey[5] , grey[6] , grey[7] );
        DownButton = Button(coord{static_cast<int>(size1.sidebarW/3) , static_cast<int>(size1.menuBarH + size1.sidebarH - (int)(12.5*size1.sidebarH / 100.0))}
                        ,dimension{static_cast<int>(size1.sidebarW/3) , static_cast<int>((int)(float)size1.sidebarH * 10.0 / 100.0) }
                       ,to_string("" , 0) , &robotoRegular
                       , grey[4] , grey[5] , grey[6] , grey[7] );
        upButton.shape.setTexture(&upButton_texture , 1 );
        DownButton.shape.setTexture(&DownButton_texture , 1 );
        bool prevShadowButtonStates[4] ;
        render_downButton = render_upButton = 1 ;
        if ( start_of_componente_sidebar == 1 )
        {
            render_upButton = 0 ;
        }
        if ( start_of_componente_sidebar == nr_of_components - 4 )
        {
            render_downButton = 0 ;
        }
        DownButton.update(sf::Vector2f(sf::Mouse::getPosition(target).x , sf::Mouse::getPosition(target).y ));
        upButton.update(sf::Vector2f(sf::Mouse::getPosition(target).x , sf::Mouse::getPosition(target).y ));
        if ( DownButton.shadowButtonState && start_of_componente_sidebar < nr_of_components - 4 )
        {
            DownButton.shadowButtonState = 0;
            ++start_of_componente_sidebar ;
        }
        if ( upButton.shadowButtonState && start_of_componente_sidebar > 1 )
        {
            upButton.shadowButtonState = 0 ;
            --start_of_componente_sidebar ;
        }
        for ( int i = 0 ; i < 4 ; ++i  )
        {
            prevShadowButtonStates[i] = Componente_sidebar[i].shadowButtonState;
            drawElement( target , i + start_of_componente_sidebar , coord {static_cast<int>(Componente_sidebar[i].shape.getPosition().x) , static_cast<int>(Componente_sidebar[i].shape.getPosition().y) }
                    , coord {static_cast<int>(Componente_sidebar[i].shape.getPosition().x + Componente_sidebar[i].shape.getSize().x) , static_cast<int>(Componente_sidebar[i].shape.getPosition().y + Componente_sidebar[i].shape.getSize().y)} );
        }
        bool ok = 0;
        anyShadowButton = 0 ;
        for( int i = 0 ; i < 4 ; ++i )
        {
        Componente_sidebar[i]= Button(coord{(int)(size1.sidebarW * 30.0 / 100.0) , static_cast<int>(size1.menuBarH+ (int)( 17.5 * size1.sidebarH / 100.0  + i* ( 16.575 * size1.sidebarH / 100.0) )) }
                                      ,dimension{(int)(40.0 * size1.sidebarW / 100.0) , (int)(14.375 * size1.sidebarH / 100.0) }
                                      ,to_string( "" , 0), &robotoRegular ,
                                       grey[4] , grey[5] , grey[6] , grey[7]  ) ;
        Componente_sidebar[i].shadowButtonState = prevShadowButtonStates[i];
        }
        for ( int i = 0 ; i < 4 ; ++i )
        {
        Componente_sidebar[i].update(sf::Vector2f(sf::Mouse::getPosition(target).x , sf::Mouse::getPosition(target).y ));
        Componente_sidebar[i].shape.setTexture(&components_texture[i + start_of_componente_sidebar ] , true );
        }
        for ( int i = 0 ; i < 4 ; ++i )
        {
            if ( Componente_sidebar[i].shadowButtonState )
            {
                anyShadowButton = 1 ;
                state.compId = i + start_of_componente_sidebar;
            }
        }
        for ( int i = 0 ; i < 4 ; ++i ) Componente_sidebar[i].render(target);
        if ( render_downButton )DownButton.render(target);
        if ( render_upButton )upButton.render(target);
    }
}

bool accessible ( coord poz , int val ) // is inside the matrix and has the same value as the variable val
{
    if ( poz.x < 1 || poz.x >= MATRIX_W || poz.y < 1 || poz.y >= MATRIX_W || occupied[poz.x][poz.y].comp_nr != val ) return 0 ;
    return 1 ;
}

coord determineTopLeftCornerOfTheOccupiedMatrix( coord poz ) //modified fiil algorithm implementation
{
    bool ok = true ;
    while ( ok )
    {
        ok = false ;
        for ( int i = 0 ; i <= 1 ; ++i )
        {
            coord vec = { poz.x + dx[i] , poz.y + dy[i] };
            if ( accessible( vec , occupied[poz.x][poz.y].comp_nr ) )
            {
                ok = true ;
                poz = vec ;
            }
        }
    }
    return poz ;

}


void moveAndDeleteInitialOccupiedMatrix ( coord poz , coord dif  ) //poz is the top left corner of the matrix corresponding to the element that is about to be moved
{
    //diff is the difference in matrix "nodes" between the position of the element and where we want to move it
    int val = occupied[poz.x][poz.y].comp_nr;
    int n = poz.x ;
    int m = poz.y ;
    while ( occupied[n+1][m].comp_nr == val ) ++n;
    while ( occupied[n][m+1].comp_nr == val ) ++m;
    create_occupied templateOccupied = occupied[poz.x][poz.y];
    for ( int x = poz.x ; x <= n ; ++x )
    {
        for (int y = poz.y ; y <= m ; ++y)
        {
            occupied[x][y] = { 0 , 0 , 0 , 0};
        }
    }for ( int x = poz.x ; x <= n ; ++x )
    {
        for (int y = poz.y ; y <= m ; ++y)
        {
            occupied[x + dif.x ][y + dif.y] = templateOccupied ;
            occupied[x + dif.x ][y + dif.y].start = coord { templateOccupied.start.x + dif.x , templateOccupied.start.y + dif.y };
        }
    }
}


void DeleteInitialOccupiedMatrix ( coord poz  ) //poz is the top left corner of the matrix corresponding to the element that is about to be moved
{
    //diff is the difference in matrix "nodes" between the position of the element and where we want to move it
    int val = occupied[poz.x][poz.y].comp_nr;
    int n = poz.x ;
    int m = poz.y ;
    while ( occupied[n+1][m].comp_nr == val ) ++n;
    while ( occupied[n][m+1].comp_nr == val ) ++m;
    for ( int x = poz.x ; x <= n ; ++x )
    {
        for (int y = poz.y ; y <= m ; ++y)
        {
            occupied[x][y] = { 0 , 0 , 0 , 0};
        }
    }
}


bool isOccupied ( coord TopLeftCorner, int xDiff , int yDiff , int k  ) // verifying if we can move in a position the element
{
    int val = occupied[TopLeftCorner.x][TopLeftCorner.y].comp_nr;
    int n = TopLeftCorner.x;
    int m = TopLeftCorner.y;
    while ( occupied[n+1][m].comp_nr == val ) ++n;
    while ( occupied[n][m+1].comp_nr == val ) ++m;
    for ( int x = TopLeftCorner.x ; x <= n ; ++x )
    {
        for ( int y = TopLeftCorner.y ; y <= m ; ++y )
        {
            if ( x > MATRIX_W || y > MATRIX_H ) return 1 ;
            if ( matrix[x][y][k].id == 1  )
            {
                if ( matrix[x+ xDiff][y + yDiff][k].id > 1 )return 1;
            }
            else
            {
                if ( occupied[x + xDiff][y + yDiff].comp_nr != 0
                    && occupied[TopLeftCorner.x][TopLeftCorner.y].comp_nr != occupied[x+xDiff][y+yDiff].comp_nr) return 1 ;
            }
        }
    }
    return 0 ;
}

void draw_move_line ( coord a , coord b  )
{
        if ( b.x > a.x )
        {
            if ( a.y != b.y)
            {
                bool ok = true ;
                int x = a.x, y = a.y ;
                while ( ok && y != b.y )
                {
                    if ( occupied[x][y].comp_nr >= 1 )
                    {
                        ok = false ;
                    }
                    else
                    {
                        if ( y > b.y) --y ;
                        else ++x;
                    }
                }
                while ( ok && x != b.x )
                {
                    if ( occupied[x][y].comp_nr >= 1)
                    {
                        ok = false ;
                    }
                    else
                    {
                        if ( x > b.x ) --x ;
                        else ++x ;
                    }
                }
                bool ok2 = false  ;
                ++nrOfDrawedComponents;


                for ( int k = 0 ; k < 4 ; ++k )
                {
                    if ( matrix[a.x][a.y][k].id == 0 )
                    {
                        ok2 = true ;
                        matrix[a.x][a.y][k].id = 1 ;
                        matrix[a.x][a.y][k].end = coord{a.x , b.y };
                        for ( int y = a.y ; y <= b.y ; ++y )
                        {
                            occupied[a.x][y].comp_nr = nrOfDrawedComponents ;
                            occupied[a.x][y].k = k ;
                            occupied[a.x][y].start = a ;
                        }
                        break ;
                    }
                }
                if ( ok2 )
                {
                    for ( int k = 0 ; k < 4 ; ++k )
                    {
                        if ( matrix[a.x][b.y][k].id == 0 )
                        {
                            for ( int x = a.x ; x <= b.x ; ++x )
                            {
                                occupied[x][b.y].comp_nr = nrOfDrawedComponents ;
                                occupied[x][b.y].k =  k;
                                occupied[x][b.y].start = coord { a.x , b.y };
                            }
                            matrix[a.x][b.y][k].id = 1 ;
                            matrix[a.x][b.y][k].end = coord{b.x , b.y };
                            break ;
                        }
                    }
                }
            }
            else
            {
                bool ok = true ;
                int x = a.x, y = a.y ;
                while ( ok && y != b.y )
                {
                    if ( occupied[x][y].comp_nr >= 1 )
                    {
                        ok = false ;
                    }
                    else
                    {
                        if ( y > b.y) --y ;
                        else ++x;
                    }
                }
                if ( ok )
                {
                    for ( int k = 0 ; k < 4 ; ++k )
                    {
                        if ( matrix[a.x][a.y][k].id == 0 )
                        {
                            matrix[a.x][a.y][k].id = 1 ;
                            matrix[a.x][a.y][k].end = coord {a.x , b.y} ;
                            for ( int y = a.y ; y <= b.y ; ++y )
                            {
                                occupied[a.x][y].comp_nr = nrOfDrawedComponents ;
                                occupied[a.x][y].k = k ;
                                occupied[a.x][y].start = a ;
                            }
                        }

                    }

                }
            }
        }
}

int main()
{
    load_components();
    start_of_componente_sidebar = 1 ;
    state.status = 0;
    // switchTheme();
    defineComponents();
    //std::cout << nr_componente << " " << componente[2].lin->a.y ;
    robotoRegular.loadFromFile( + "Roboto-Regular.ttf");
    size1.W = sf::VideoMode::getDesktopMode().width * 3/4; // Initial width of the window is 75% of the screen
    sf::RenderWindow window(sf::VideoMode(size1.W, size1.W / 2), "Electron", sf::Style::Default); // Initialize the window
    updateSizes(window); // Generate initial sizes accordingly
    render(window); // Render all the content
    window.display();
    sf::Vector2i vec;
    loadTextures();
    coord initialMoveCoordonates = { 0 , 0 };
    bool isMoving;
    for ( int i =  0 ; i < 4 ; ++i )
    {
        Componente_sidebar[i].shadowButtonState = 0 ;
    }
    while(window.isOpen())
    {
        usleep(10);
        sf::Event event;
        while(window.pollEvent(event))
        {
            usleep(10);
            switch(event.type)
            {
                window.setMouseCursorVisible(1);
                case sf::Event::MouseButtonPressed :
                    {
                        normalButtonCursor.update(sf::Vector2f(sf::Mouse::getPosition(window).x , sf::Mouse::getPosition(window).y));
                        if ( normalButtonCursor.buttonState == BTN_PRESSED )
                        {
                            Curent_Cursor_state = NORMAL_CURSOR ;
                            move_state = 0 ;
                            text_state = 0 ;
                            delete_state = 0 ;
                            state.compId = 0;
                        }
                        moveButtonCursor.update(sf::Vector2f(sf::Mouse::getPosition(window).x , sf::Mouse::getPosition(window).y));
                        if ( moveButtonCursor.buttonState == BTN_PRESSED )
                        {
                            if ( move_state )
                            {
                                state.compId = 0 ;
                                Curent_Cursor_state = NORMAL_CURSOR ;
                                move_state = 0 ;
                            }
                            else
                            {
                                text_state = 0 ;
                                delete_state = 0 ;
                                state.compId = 0 ;
                                move_state = 1;
                                Curent_Cursor_state = MOVE_CURSOR;
                            }
                            usleep(50);
                        }
                        textButton.update(sf::Vector2f(sf::Mouse::getPosition(window).x , sf::Mouse::getPosition(window).y));
                        if ( textButton.buttonState == BTN_PRESSED )
                        {
                            if ( text_state )
                            {
                                state.compId = 0 ;
                                Curent_Cursor_state = NORMAL_CURSOR ;
                                text_state = 0 ;
                            }
                            else
                            {
                                delete_state = 0 ;
                                move_state = 0 ;
                                state.compId = 0 ;
                                text_state = 1;
                                Curent_Cursor_state = TEXT_CURSOR;
                            }
                            usleep(50);
                        }

                        deleteButtonCursor.update(sf::Vector2f(sf::Mouse::getPosition(window).x , sf::Mouse::getPosition(window).y));
                        if ( deleteButtonCursor.buttonState == BTN_PRESSED )
                        {
                            if ( delete_state )
                            {
                                state.compId = 0 ;
                                Curent_Cursor_state = NORMAL_CURSOR ;
                                delete_state = 0 ;
                            }
                            else
                            {
                                delete_state = 1 ;
                                move_state = 0 ;
                                state.compId = 0 ;
                                text_state = 0;
                                Curent_Cursor_state = DELETE_CURSOR;
                            }
                            usleep(50);
                        }


                        DorNButton.update(sf::Vector2f(sf::Mouse::getPosition(window).x , sf::Mouse::getPosition(window).y ));
                        if ( DorNButton.buttonState == BTN_PRESSED )
                        {
                            switchTheme();
                        }

                        saveButton.update(sf::Vector2f(sf::Mouse::getPosition(window).x , sf::Mouse::getPosition(window).y ));
                        if(saveButton.buttonState == BTN_PRESSED )
                        {
                            download();
                        }

                        loadButton.update( sf::Vector2f(sf::Mouse::getPosition(window).x , sf::Mouse::getPosition(window).y ));
                        if(loadButton.buttonState == BTN_PRESSED )
                        {
                            upload(window);
                        }

                        if ( anyShadowButton && sf::Mouse::getPosition(window).x >= size1.sidebarW && sf::Mouse::getPosition(window).x <= size1.W
                            && sf::Mouse::getPosition(window).y >= size1.menuBarH && sf::Mouse::getPosition(window).y <= size1.H - size1.footerBarH )
                        //inside the canvas and we have a component selected
                        {
                            auxDemo = shadowButton;
                        }
                        coord MouseCoordanates = coord { sf::Mouse::getPosition(window).x , sf::Mouse::getPosition(window).y };
                        coord matrixMouseCoordanates = pxToMatrix(MouseCoordanates ) ; // x stand for width , y for height
                        if ( sf::Mouse::getPosition(window).x >= size1.sidebarW && sf::Mouse::getPosition(window).x <= size1.W
                            && sf::Mouse::getPosition(window).y >= size1.menuBarH && sf::Mouse::getPosition(window).y <= size1.H - size1.footerBarH )
                        //inside the canvas and we have a component selected
                        {
                            //std::cout <<isEndHere[matrixMouseCoordanates.x][matrixMouseCoordanates.y] <<'\n' ;
                        }
                        if ( move_state )
                        {
                            usleep(10);
                            if (sf::Mouse::getPosition(window).x >= size1.sidebarW && sf::Mouse::getPosition(window).x <= size1.W
                            && sf::Mouse::getPosition(window).y >= size1.menuBarH && sf::Mouse::getPosition(window).y <= size1.H - size1.footerBarH )//inside the cavas
                            {
                                occupied[matrixMouseCoordanates.x][matrixMouseCoordanates.y].populated ;
                                isMoving = 1 ;
                                initialMoveCoordonates = matrixMouseCoordanates ;
                            }
                        }
                        else
                        {
                            if ( delete_state )
                            {
                                if (sf::Mouse::getPosition(window).x >= size1.sidebarW && sf::Mouse::getPosition(window).x <= size1.W
                            && sf::Mouse::getPosition(window).y >= size1.menuBarH && sf::Mouse::getPosition(window).y <= size1.H - size1.footerBarH )
                                    if ( occupied[matrixMouseCoordanates.x][matrixMouseCoordanates.y].comp_nr >= 1 )
                                    {
                                        coord start = occupied[matrixMouseCoordanates.x][matrixMouseCoordanates.y].start ;
                                        for ( int k = 0 ; k < 4 ; ++k)
                                        {
                                            if ( abs(matrixMouseCoordanates.x - start.x ) <= 1 ) // the occupied matrix is vertical
                                            {
                                                if ( abs ( matrix[start.x][start.y][k].end.x - start.x) <= 1 ) // the component corresponding to k is also vertical
                                                {
                                                    if ( ( matrixMouseCoordanates.y > start.y && matrix[start.x][start.y][k].end.y > start.y ) || ( matrixMouseCoordanates.y < start.y && matrix[start.x][start.y][k].end.y < start.y ) )
                                                    {
                                                        isEndHere[start.x][start.y] = 0 ;
                                                        isEndHere[matrix[start.x][start.y][k].end.x][matrix[start.x][start.y][k].end.y] = 0 ;
                                                        matrix[start.x][start.y][k].end.x = 0;
                                                        matrix[start.x][start.y][k].end.y = 0;
                                                        matrix[start.x][start.y][k].id = 0 ;
                                                        matrix[start.x][start.y][k].invisible = 0 ;
                                                        matrix[start.x][start.y][k].text[0] = NULL ;
                                                        DeleteInitialOccupiedMatrix( determineTopLeftCornerOfTheOccupiedMatrix( matrixMouseCoordanates ) ) ;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if ( abs(matrixMouseCoordanates.y - start.y ) <= 1 ) // the occupied matrix is horizontal
                                                {
                                                    if ( abs ( matrix[start.x][start.y][k].end.y - start.y) <= 1 ) // the component corresponding to k is also horizontal
                                                    {
                                                        if ( ( matrixMouseCoordanates.x > start.x && matrix[start.x][start.y][k].end.x > start.x ) || ( matrixMouseCoordanates.x < start.x && matrix[start.x][start.y][k].end.x < start.x ) )
                                                        {
                                                            isEndHere[start.x][start.y] = 0 ;
                                                            isEndHere[matrix[start.x][start.y][k].end.x][matrix[start.x][start.y][k].end.y] = 0 ;
                                                            matrix[start.x][start.y][k].end.x = 0;
                                                            matrix[start.x][start.y][k].end.y = 0;
                                                            matrix[start.x][start.y][k].id = 0 ;
                                                            matrix[start.x][start.y][k].invisible = 0 ;
                                                            matrix[start.x][start.y][k].text[0] = NULL ;
                                                            DeleteInitialOccupiedMatrix( determineTopLeftCornerOfTheOccupiedMatrix( matrixMouseCoordanates ) ) ;
                                                        }
                                                    }
                                                }
                                            }
                                        }

                                    }

                            }
                            else
                            {
                                if(text_state)
                                {
                                     mouseClicked(window);
                                }
                            }
                        }
                        if (!move_state  && !delete_state &&!text_state) mouseClicked(window);
                        render(window);
                        window.display();

                        break;
                    }
                case sf::Event::MouseButtonReleased:
                    {
                        if ( move_state && isMoving  )
                        {
                            usleep(10);
                            isMoving = 0 ;
                            if ( sf::Mouse::getPosition(window).x >= size1.sidebarW && sf::Mouse::getPosition(window).x <= size1.W
                            && sf::Mouse::getPosition(window).y >= size1.menuBarH && sf::Mouse::getPosition(window).y <= size1.H - size1.footerBarH )
                            {
                                coord matrixMouseCoordonates = coord { sf::Mouse::getPosition(window).x , sf::Mouse::getPosition(window).y};
                                coord afterMoveCoordonates  = pxToMatrix( matrixMouseCoordonates ) ;

                                int xDifference = afterMoveCoordonates.x - initialMoveCoordonates.x;
                                int yDifference = afterMoveCoordonates.y - initialMoveCoordonates.y;

                                int x = occupied[initialMoveCoordonates.x][initialMoveCoordonates.y].start.x ;
                                int y = occupied[initialMoveCoordonates.x][initialMoveCoordonates.y].start.y ;
                                int k = occupied[initialMoveCoordonates.x][initialMoveCoordonates.y].k ;

                                coord initialStart = occupied[initialMoveCoordonates.x][initialMoveCoordonates.y].start;
                                coord initialEnd = matrix[initialStart.x][initialStart.y][k].end ;
                                coord currentStart = coord{initialStart.x + xDifference , initialStart.y + yDifference};
                                coord currentEnd = coord { initialEnd.x + xDifference , initialEnd.y + yDifference };


                                coord topLeftCorner; //top left corner of the initial member in the occupied matrix
                                topLeftCorner = determineTopLeftCornerOfTheOccupiedMatrix(initialMoveCoordonates);
                                if ( !isOccupied(topLeftCorner , xDifference , yDifference , k ) &&  !( xDifference == 0 && yDifference ==0 ) )
                                {

                                    matrix[x + xDifference][y+yDifference][k] = matrix[x][y][k];
                                    matrix[x + xDifference][y+yDifference][k].end.x += xDifference;
                                    matrix[x + xDifference][y+yDifference][k].end.y += yDifference;


                                    moveAndDeleteInitialOccupiedMatrix( topLeftCorner , coord { xDifference , yDifference } );



                                    matrix[x][y][k].end = {0 , 0 };
                                    matrix[x][y][k].id = 0 ;
                                    matrix[x][y][k].invisible = 0 ;
                                    matrix[x][y][k].text[0]= NULL;

                                    isEndHere[currentStart.x][currentStart.y] = 1 ;
                                    isEndHere[currentEnd.x][currentEnd.y] = 1 ;

                                    if( isEndHere[initialStart.x][initialStart.y] )
                                    {
                                        //draw_move_line( initialStart , currentStart );
                                    }
                                    isEndHere[initialStart.x][initialStart.y] = 0 ;
                                    isEndHere[initialEnd.x][initialEnd.y] = 0;


                                }
                            }
                        }
                        render(window);
                        window.display();
                        break;
                    }
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::Resized: // If the window has been resized
                    {
                    updateSizes(window);
                    sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
                    window.setView(sf::View(visibleArea));
                    window.clear();
                    render(window);
                    window.display();
                    vec.x = size1.sidebarW;
                    vec.y = size1.menuBarH;
                    //sf::Mouse::setPosition(vec , window );
                    //mouseMoved(window );
                    //highlightDot(window, sf::Mouse::getPosition(window).x , sf::Mouse::getPosition(window).y );
                    //generateMenuBar(window, &robotoRegular, menuBar, buttonNew, buttonUndo, buttonRedo);
                    //drawAndDisplayMenuBar(window, menuBar, buttonNew, buttonUndo, buttonRedo);
                    break;
                    }
                case sf::Event::MouseMoved:
                    {
                        mouseMoved(window);
                        break;
                    }

                default:
                    break;
            }
        }
    }
}


void render(sf::RenderWindow& target)
{
    generateBackground(target); // Draw the background color
    generateMenuBar(target); // Generate the menu bar
    generateSidebar(target); // Generate the sidebar
    generateCanvas(target); // Generate the canvas
    generateFooterBar(target); // Generate the footer bar
    drawElements(target);
}
