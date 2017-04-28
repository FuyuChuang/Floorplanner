/****************************************************************************
  FileName  [ floorplanner.cpp ]
  Synopsis  [ Implementation of the floorplanner based on b*-tree. ]
  Author    [ Fu-Yu Chuang ]
  Date      [ 2017.4.27 ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <climits>
#include <iterator>
#include <cmath>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "floorplanner.h"
using namespace std;
using namespace cv;

double Floorplanner::getHPWL() const
{
    double HPWL = 0;
    for (size_t i = 0, end = _netList.size(); i < end; ++i) {
        HPWL += _netList[i]->calcHPWL();
    }
    return (HPWL / 2.0);
}

double Floorplanner::getCost(BStarTree& tree)
{
    double cost = 0;
    this->packTree(tree);
    size_t maxX = Block::getMaxX();
    size_t maxY = Block::getMaxY();

    if (maxX > _width)
        cost += 1.0e8 * (maxX - _width);
    if (maxY > _height)
        cost += 1.0e8 * (maxY - _height);

    cost += _alpha * maxX * maxY;
    // cost += (1 - _alpha) * this->getHPWL();
    // cout << this->getHPWL() << endl;
    /*
    for (size_t i = 0, end = _blockList.size(); i < end; ++i) {
        cout << _blockList[i]->getX1() << " " << _blockList[i]->getY1() << " "
             << _blockList[i]->getX2() << " " << _blockList[i]->getY2() << endl;
    }
    */
    return cost;
}

size_t Floorplanner::getModuleArea() const
{
    size_t area = 0;
    for (size_t i = 0, end = _blockList.size(); i < end; ++i) {
        area += _blockList[i]->getArea();
    }
    return area;
}

void Floorplanner::readCircuit(fstream& inBlk, fstream& inNet)
{
    this->readBlock(inBlk);
    this->readNet(inNet);

    return;
}

void Floorplanner::floorplan()
{
    srand(time(NULL));
    _start = clock();
    BStarTree tmpTree = _bestTree;

    double cost = this->getCost(tmpTree), accCost = 0;
    double r = 0.9, p = 0.99;
    bool fit = false;

    for (size_t i = 0; i < 100; ++i) {
        vector<BStarTree> trees = tmpTree.perturb();
        size_t best = this->selectBestTree(trees, fit);
        double newCost = this->getCost(trees[best]);
        tmpTree = trees[best];
        accCost += (newCost - cost);
        cost = newCost;
    }

    double T = abs((accCost / 100) / log(p));
    cost = getCost(tmpTree);
    _bestTree = tmpTree;
    double tmpCost = cost;
    size_t P = 3000;

    while (T > 1.0) {
        for (size_t i = 0; i < P; ++i) {
            vector<BStarTree> trees = tmpTree.perturb();
            size_t best = this->selectBestTree(trees, fit);
            double newCost = this->getCost(trees[best]);
            if (!fit && this->checkFit()) fit = true;
            if (fit && !this->checkFit()) continue;
            double delta = newCost - cost;
            if (delta <= 0 || (double)rand() / RAND_MAX < exp(-1 * delta / T)) {
                tmpTree = trees[best];
                cost = newCost;
                if (cost < tmpCost) {
                    _bestTree = tmpTree;
                    tmpCost = cost;
                }
            }
        }
        T *= r;
        cout << "T = " << T << ", cost = " << cost << "       \r";
        cout.flush();
    }
    _stop = clock();

    cost = this->getCost(_bestTree);

    // opencv drawing
    // image(row, column, channel)
    // Mat image(_height, _width, CV_8UC3);
    Mat image;
    size_t maxY = (Block::getMaxY() > _height)? Block::getMaxY(): _height;
    size_t maxX = (Block::getMaxX() > _width)? Block::getMaxX(): _width;
    if (fit) {
        image = Mat(_height, _width, CV_8UC3);
    }
    else {
        image = Mat(maxY, maxX, CV_8UC3);
    }
    size_t height = (_height > Block::getMaxY())? _height: Block::getMaxY();
    image.setTo(Scalar(255, 255, 255));
    cout << endl;
    for (size_t i = 0, end = _blockList.size(); i < end; ++i) {
        size_t x1 = _blockList[i]->getX1();
        size_t y1 = height - _blockList[i]->getY1();
        size_t x2 = _blockList[i]->getX2();
        size_t y2 = height - _blockList[i]->getY2();
        rectangle(image, Point(x1, y1), Point(x2, y2), Scalar(0, 255, 255), -1, 8);
        rectangle(image, Point(x1, y1), Point(x2, y2), Scalar(0, 128, 0), 3, 8);
        cout << _blockList[i]->getName() << " " << _blockList[i]->getX1() << " "
             << _blockList[i]->getY1() << " " << _blockList[i]->getX2() << " "
             << _blockList[i]->getY2() << endl;
        putText(image, _blockList[i]->getName(), Point(x1 + 3, y1 - 5), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 128, 0));
    }
    if (!fit) {
        rectangle(image, Point(0, maxY), Point(_width, maxY -_height), Scalar(0, 0, 255), 5, 8);
    }
    imwrite("floorplan.jpg", image);

    return;
}

void Floorplanner::packTree(BStarTree& tree)
{
    _contourList.clear();
    _contourList.push_back(new LNode());
    _contourList.back()->setPos(0, 0);
    _contourList.push_back(new LNode());
    _contourList.back()->setPos(INT_MAX, 0);
    LNode* head = _contourList[0];
    head->insertNext(_contourList[1]);
    Block::setMaxX(0);
    Block::setMaxY(0);
    this->packBlock(tree._root, head);
    for (size_t i = 0, end = _contourList.size(); i < end; ++i) {
        delete _contourList[i];
    }
    return;
}

bool Floorplanner::checkFit()
{
    return ((Block::getMaxX() <= _width) && (Block::getMaxY() <= _height));
}

size_t Floorplanner::selectBestTree(vector<BStarTree>& trees, bool fit)
{
    double bestCost = this->getCost(trees[0]);
    size_t best = 0;
    for (size_t i = 1, end = trees.size(); i < end; ++i) {
        double cost = this->getCost(trees[i]);
        if (fit && !this->checkFit()) continue;
        if (cost < bestCost) {
            cost = bestCost;
            best = i;
        }
    }
    return best;
}

void Floorplanner::printSummary() const
{
    double wireLength = this->getHPWL();
    double area = this->getArea();
    cout << endl;
    cout << "==================== Summary ====================" << endl;
    cout << " Cost: "   << fixed << _alpha * area + (1 - _alpha) * wireLength << endl;
    cout << " Wire: "   << fixed << wireLength << endl;
    cout << " Area: "   << fixed << area << endl;
    cout << " Width: "  << Block::getMaxX() << " (limit = " << _width << ")" << endl;
    cout << " Height: " << Block::getMaxY() << " (limit = " << _height << ")" << endl;
    cout << " Dead space: " << fixed << (area - this->getModuleArea()) << endl;
    cout << " Time: "   << (double)(_stop - _start) / CLOCKS_PER_SEC << " secs" << endl;
    cout << "=================================================" << endl;
    return;
}

void Floorplanner::reportBlock() const
{
    // TODO: report position
    assert(_blockNum == _blockList.size());
    cout << "Number of blocks: " << _blockNum << endl;
    for (size_t i = 0, end = _blockList.size(); i < end; ++i) {
        cout << setw(8) << _blockList[i]->getName() << setw(6)
             << _blockList[i]->getWidth() << setw(6) << _blockList[i]->getHeight();
        cout << endl;
    }
    return;
}

void Floorplanner::reportTerm() const
{
    assert(_termNum == _termList.size());
    cout << "Number of terminals: " << _termNum << endl;
    for (size_t i = 0, end = _termList.size(); i < end; ++i) {
        cout << setw(8) << _termList[i]->getName() << setw(6)
             << _termList[i]->getX1() << setw(6) << _termList[i]->getY1();
        cout << endl;
    }

    return;
}

void Floorplanner::reportNet() const
{
    assert(_netNum == _netList.size());
    cout << "Number of nets: " << _netNum << endl;
    for (size_t i = 0, end_i = _netList.size(); i < end_i; ++i) {
        const vector<Terminal*> termList = _netList[i]->getTermList();
        for (size_t j = 0, end_j = termList.size(); j < end_j; ++j) {
            cout << setw(6) << termList[j]->getName();
        }
        cout << endl;
    }

    return;
}

void Floorplanner::writeResult(fstream& outFile)
{
    stringstream buff;
    double wireLength = this->getHPWL();
    double area = this->getArea();

    // <final cost>
    outFile << fixed << (_alpha * area + (1 - _alpha) * wireLength) << '\n';

    // <total wirelength>
    outFile << fixed << wireLength << '\n';

    // <chip_area>
    outFile << fixed << area << '\n';

    // <chip_width> <chip_height>
    buff << Block::getMaxX();
    outFile << buff.str() << " ";
    buff.str("");
    buff << Block::getMaxY();
    outFile << buff.str() << '\n';
    buff.str("");

    // <program_runtime>
    buff << (double)(_stop - _start) / CLOCKS_PER_SEC;
    outFile << buff.str() << '\n';
    buff.str("");

    // <macro_name> <x1> <y1> <x2> <y2>
    for (size_t i = 0, end = _blockList.size(); i < end; ++i) {
        outFile << _blockList[i]->getName() << " "
                << _blockList[i]->getX1() << " " << _blockList[i]->getY1() << " "
                << _blockList[i]->getX2() << " " << _blockList[i]->getY2() << '\n';
    }

    return;
}

// private member functions
void Floorplanner::readBlock(fstream& inBlk)
{
    string str;

    // Outline: <outline width, outline height>
    inBlk >> str;
    assert(str == "Outline:");
    inBlk >> str;
    _width = stoi(str);
    inBlk >> str;
    _height = stoi(str);

    // NumBlocks: <# of blocks>
    inBlk >> str;
    assert(str == "NumBlocks:");
    inBlk >> str;
    _blockNum = stoi(str);

    // NumTerminals: <# of terminals>
    inBlk >> str;
    assert(str == "NumTerminals:");
    inBlk >> str;
    _termNum = stoi(str);

    // read macros
    // <macro name> <macro width> <macro height>
    for (size_t i = 0; i < _blockNum; ++i) {
        string name;
        size_t width, height;
        inBlk >> name;
        inBlk >> str;
        width = stoi(str);
        inBlk >> str;
        height = stoi(str);
        _blockList.push_back(new Block(name, width, height));
        _termName2Ptr[name] = _blockList.back();
    }

    // read terminals
    // <terminal name> terminal <x coordinate> <y coordinate>
    for (size_t i = 0; i < _termNum; ++i) {
        string name;
        size_t x, y;
        inBlk >> name;
        inBlk >> str;
        inBlk >> str;
        x = stoi(str);
        inBlk >> str;
        y = stoi(str);
        _termList.push_back(new Terminal(name, x, y));
        _termName2Ptr[name] = _termList.back();
    }

    return;
}

void Floorplanner::readNet(fstream& inNet)
{
    string str;

    // NumNets: <# of nets>
    inNet >> str;
    assert(str == "NumNets:");
    inNet >> str;
    _netNum = stoi(str);

    // read nets
    // NetDegree: <# of terminals in this net>
    // <terminal name> ...
    for (size_t i = 0; i < _netNum; ++i) {
        size_t termNum;
        inNet >> str;
        assert(str == "NetDegree:");
        inNet >> str;
        termNum = stoi(str);
        _netList.push_back(new Net());
        for (size_t j = 0; j < termNum; ++j) {
            inNet >> str;
            _netList.back()->addTerm(_termName2Ptr[str]);
        }
    }

    return;
}

void Floorplanner::packBlock(TNode* node, LNode* head)
{
    // cout << node->getId() << " ";
    Block* block = _blockList[node->getId()];
    size_t x = head->_x;
    size_t prevY = head->_y, maxY = head->_y;
    size_t width = block->getWidth(node->getOrient());
    size_t height = block->getHeight(node->getOrient());
    while (x + width > head->_next->_x) {
        prevY = head->_next->_y;
        maxY = (maxY > prevY)? maxY: prevY;
        head->deleteNext();
    }
    block->setPos(x, maxY, x + width, maxY + height);
    if (x + width > Block::getMaxX())
        Block::setMaxX(x + width);
    if (maxY + height > Block::getMaxY())
        Block::setMaxY(maxY + height);
    head->_y = maxY + height;
    if (x + width < head->_next->_x) {
        _contourList.push_back(new LNode());
        _contourList.back()->setPos(x + width, prevY);
        head->insertNext(_contourList.back());
    }
    if (node->_left != NULL)
        packBlock(node->_left, head->_next);
    if (node->_right != NULL)
        packBlock(node->_right, head);
    return;
}
