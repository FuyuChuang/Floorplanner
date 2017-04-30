/****************************************************************************
  FileName  [ module.h ]
  Synopsis  [ Define the data structures for the components on the chip. ]
  Author    [ Fu-Yu Chuang ]
  Date      [ 2017.4.24 ]
****************************************************************************/
#ifndef MODULE_H
#define MODULE_H

#include <vector>
#include <string>
using namespace std;

class Terminal
{
public:
    // constructor and destructor
    Terminal(string& name, size_t x, size_t y) :
        _name(name), _x1(x), _y1(y), _x2(x), _y2(y) { }
    ~Terminal()  { }

    // basic access methods
    const string getName()  { return _name; }
    const size_t getX1()    { return _x1; }
    const size_t getX2()    { return _x2; }
    const size_t getY1()    { return _y1; }
    const size_t getY2()    { return _y2; }

    // set functions
    void setName(string& name) { _name = name; }
    void setPos(size_t x1, size_t y1, size_t x2, size_t y2) {
        _x1 = x1;   _y1 = y1;
        _x2 = x2;   _y2 = y2;
    }

protected:
    string      _name;      // module name
    size_t      _x1;        // min x coordinate of the terminal
    size_t      _y1;        // min y coordinate of the terminal
    size_t      _x2;        // max x coordinate of the terminal
    size_t      _y2;        // max y coordinate of the terminal
};


class Block : public Terminal
{
public:
    // constructor and destructor
    Block(string& name, size_t w, size_t h) :
        Terminal(name, 0, 0), _w(w), _h(h) { }
    ~Block() { }

    // basic access methods
    const size_t getWidth(bool rotate = false)  { return rotate? _h: _w; }
    const size_t getHeight(bool rotate = false) { return rotate? _w: _h; }
    const size_t getArea()  { return _h * _w; }
    static size_t getMaxX() { return _maxX; }
    static size_t getMaxY() { return _maxY; }

    // set functions
    void setWidth(size_t w)         { _w = w; }
    void setHeight(size_t h)        { _h = h; }
    static void setMaxX(size_t x)   { _maxX = x; }
    static void setMaxY(size_t y)   { _maxY = y; }


private:
    size_t          _w;         // width of the block
    size_t          _h;         // height of the block
    static size_t   _maxX;      // maximum x coordinate for all blocks
    static size_t   _maxY;      // maximum y coordinate for all blocks
};


class Net
{
public:
    // constructor and destructor
    Net()   { }
    ~Net()  { }

    // basic access methods
    const vector<Terminal*> getTermList()   { return _termList; }

    // modify methods
    void addTerm(Terminal* term) { _termList.push_back(term); }

    // other member functions
    double calcHPWL();

private:
    vector<Terminal*>   _termList;  // list of terminals the net is connected to
};

#endif  // MODULE_H
