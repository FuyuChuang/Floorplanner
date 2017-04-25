/****************************************************************************
  FileName  [ module.cpp ]
  Synopsis  [ Define class Terminal, Block, Net member functions. ]
  Author    [ Fu-Yu Chuang ]
  Date      [ 2017.4.24 ]
****************************************************************************/
#include <climits>
#include "module.h"
using namespace std;

size_t Block::_maxX = 0;
size_t Block::_maxY = 0;

/*************************************/
/*  class Terminal member functions  */
/*************************************/


/*************************************/
/*   class Block member functions    */
/*************************************/


/*************************************/
/*    class Net member functions     */
/*************************************/
size_t Net::calcHPWL()
{
    size_t minX = INT_MAX, minY = INT_MAX, maxX = 0, maxY = 0;
    for (size_t i = 0, end = _termList.size(); i < end; ++i) {
        size_t x = _termList[i]->getX1() + _termList[i]->getX2();
        size_t y = _termList[i]->getY1() + _termList[i]->getY2();
        minX = (x < minX)? x: minX;
        minY = (y < minY)? x: minY;
        maxX = (x > maxX)? x: maxX;
        maxY = (y > maxY)? y: maxY;
    }
    return (maxX - minX) + (maxY - minY);
}
