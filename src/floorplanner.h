/****************************************************************************
  FileName  [ floorplanner.h ]
  Synopsis  [ Define an interface for floorplanning based on b*-tree. ]
  Author    [ Fu-Yu Chuang ]
  Date      [ 2017.4.27 ]
****************************************************************************/
#include <string>
#include <vector>
#include <fstream>
#include <climits>
#include <map>
#include "module.h"
#include "bStarTree.h"
using namespace std;

// Linked list node
class LNode
{
    friend class Floorplanner;

private:
    // constructor and destructor
    LNode(LNode* next = NULL) :
        _next(next) { }
    ~LNode()    { }

    void setPos(size_t x, size_t y) {
        _x = x; _y = y;
    }

    void insertNext(LNode* node) {
        LNode* n = _next;
        _next = node;
        node->_next = n;
    }

    void deleteNext() {
        _next = _next->_next;
    }

    LNode*      _next;      // next linked list node
    size_t      _x;         // coordinate x
    size_t      _y;         // coordinate y
};

class Floorplanner
{
public:
    // constructor and destructor
    Floorplanner(fstream& inBlk, fstream& inNet) :
        _start(0), _stop(0) {
        readCircuit(inBlk, inNet);
        _bestTree = BStarTree(_blockList);
        _maxLengthX = 0;
        _maxLengthY = 0;
        _minLengthX = INT_MAX;
        _minLengthY = INT_MAX;
    }
    ~Floorplanner() { }

    // basic access methods
    double getAlpha() const     { return _alpha; }
    size_t getWidth() const     { return _width; }
    size_t getHeight() const    { return _height; }
    size_t getBlockNum() const  { return _blockNum; }
    size_t getTermNum() const   { return _termNum; }
    size_t getNetNum() const    { return _width; }

    size_t getArea() const      { return Block::getMaxX() * Block::getMaxY(); }
    double getHPWL() const;
    // getting the cost inside the program, rather than the cost reported
    double getCost(BStarTree& tree);
    size_t getModuleArea() const;

    // set functions
    void setAlpha(double alpha) { _alpha = alpha; }

    // modify methods
    void readCircuit(fstream& inBlk, fstream& inNet);
    void floorplan();
    void packTree(BStarTree& tree);
    bool checkFit();
    size_t selectBestTree(vector<BStarTree>& trees, bool fit);

    // member functions about reporting
    void printSummary() const;
    void reportBlock()  const;
    void reportTerm()   const;
    void reportNet()    const;
    void writeResult(fstream& outFile);
    void drawFloorplan(BStarTree& tree);

private:
    double              _alpha;         // cost weight of bbox and area
    size_t              _width;         // chip width limit
    size_t              _height;        // chip height limit
    size_t              _blockNum;      // number of blocks
    size_t              _termNum;       // number of terminals
    size_t              _netNum;        // number of nets
    clock_t             _start;         // starting time
    clock_t             _stop;          // stopping time
    BStarTree           _bestTree;      // best B*-tree
    vector<LNode*>      _contourList;   // list of contour
    vector<Block*>      _blockList;     // list of blocks
    vector<Terminal*>   _termList;      // list of terminals
    vector<Net*>        _netList;       // list of nets

    map<string, Terminal*>  _termName2Ptr;  // mapping from terminal name to its pointer

    // data members for computing cost
    double              _avgArea;
    double              _avgWire;
    double              _maxLengthX;
    double              _minLengthX;
    double              _maxLengthY;
    double              _minLengthY;
    double              _lengthX;
    double              _lengthY;

    // private member functions
    BStarTree floorplanSA();
    void packBlock(TNode* node, LNode* head);

    void readBlock(fstream& inBlk);
    void readNet(fstream& inNet);

};

