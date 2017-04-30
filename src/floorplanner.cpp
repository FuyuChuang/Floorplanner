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
    return HPWL;
}

double Floorplanner::getCost(BStarTree& tree)
{
    double cost = 0;
    this->packTree(tree);
    size_t maxX = Block::getMaxX();
    size_t maxY = Block::getMaxY();

    // fit in width is harder than fit in height...
    if (maxX > _width)
        cost += 1.0e10 * (maxX - _width) / _lengthX;
    if (maxY > _height)
        cost += 1.0e8 * (maxY - _height) / _lengthY;
    // cost += 1.0e2 * ((maxX * maxY) - this->getModuleArea()) / _avgArea;
    // if (this->checkFit())
    // cost += 1.0e10 * abs((double(_width) / _height) - (double(maxX) / maxY));

    cost += _alpha * maxX * maxY / _avgArea;
    cost += (1 - _alpha) * this->getHPWL() / _avgWire;
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
    double bestCost = this->getCost(_bestTree);
    bool fit = this->checkFit();
    int trial = 0;
    _start = clock();
    while (!fit) {
        ++trial;
        cout << "Trial #" << trial << endl;
        BStarTree tmpBestTree = this->floorplanSA();
        double cost = this->getCost(tmpBestTree);
        fit = this->checkFit();
        if (bestCost > cost) {
            _bestTree = tmpBestTree;
            bestCost = cost;
        }
    }
    _stop = clock();
    this->packTree(_bestTree);
    this->drawFloorplan(_bestTree);


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

void Floorplanner::drawFloorplan(BStarTree& tree)
{
    // opencv drawing
    // image(row, column, channel)
    this->packTree(tree);
    Mat image;
    size_t maxY = (Block::getMaxY() > _height)? Block::getMaxY(): _height;
    size_t maxX = (Block::getMaxX() > _width)? Block::getMaxX(): _width;
    if (this->checkFit()) {
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
        putText(image, _blockList[i]->getName(), Point(x1 + 3, y1 - 5), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 128, 0));
    }
    if (!this->checkFit()) {
        rectangle(image, Point(0, maxY), Point(_width, maxY -_height), Scalar(0, 0, 255), 5, 8);
    }
    imwrite("floorplan.jpg", image);

    return;
}

BStarTree Floorplanner::floorplanSA()
{
    srand(time(NULL));

    // setup trees and costs
    BStarTree prevTree = BStarTree(_blockList);
    BStarTree tmpBestTree = BStarTree(_blockList);
    double prevCost = this->getCost(prevTree);
    double tmpBestCost = prevCost;

    // setup parameters for annealing
    double accArea = 0, accWire = 0;
    _maxLengthX = _maxLengthY = 0;
    _minLengthX = _minLengthY = INT_MAX;
    for (size_t i = 0; i < 1000; ++i) {
        vector<BStarTree> trees = prevTree.perturb();
        prevTree = trees[0];
        this->packTree(prevTree);
        accArea += this->getArea();
        accWire += this->getHPWL();
        _maxLengthX = (_maxLengthX > Block::getMaxX())? _maxLengthX: Block::getMaxX();
        _maxLengthY = (_maxLengthY > Block::getMaxY())? _maxLengthY: Block::getMaxY();
        _minLengthX = (_minLengthX < Block::getMaxX())? _minLengthX: Block::getMaxX();
        _minLengthY = (_minLengthY < Block::getMaxY())? _minLengthY: Block::getMaxY();
    }
    _avgArea = accArea / 1000;
    _avgWire = accWire / 1000;
    _lengthX = _maxLengthX - _minLengthX;
    _lengthY = _maxLengthY - _minLengthY;

    bool fit = this->checkFit();
    double accCost = 0, acc = 0;
    double r = 0.90, p = 0.98;
    size_t P = _blockList.size() * 100;
    for (size_t i = 0; i < 300; ++i) {
        vector<BStarTree> trees = prevTree.perturb();
        size_t best = this->selectBestTree(trees, fit);
        if (this->checkFit())
            fit = true;
        double newCost = this->getCost(trees[best]);
        double delta = newCost - prevCost;
        if (delta > 0) {
            accCost += delta;
            acc += 1;
        }
        prevTree = trees[best];
        prevCost = newCost;
    }

    double T = abs((accCost/acc) / log(p));
    size_t count = 0;

    // simulated annealing
    while (T > 1.0) {
        ++count;
        // for each temperature, find P neighbors
        for (size_t i = 0; i < P; ++i) {
            vector<BStarTree> trees = prevTree.perturb();
            size_t best = this->selectBestTree(trees, fit);
            double newCost = this->getCost(trees[best]);
            double delta = newCost - prevCost;
            if (this->checkFit())
                fit = true;
            // downhill move
            if (delta <= 0) {
                prevTree = trees[best];
                prevCost = newCost;
                if (prevCost < tmpBestCost) {
                    tmpBestTree = prevTree;
                    tmpBestCost = prevCost;
                }
            }
            // uphill move
            else if (((double)rand() / RAND_MAX) < exp(-1 * delta / T)) {
                prevTree = trees[best];
                prevCost = newCost;
            }
            else {
                // do not accept this neighbor tree
            }
        }
        cout << fixed << setprecision(2) << "T = " << T << ", cost = " << tmpBestCost << "       \r";
        cout.flush();
        T *= r;
    }

    this->drawFloorplan(tmpBestTree);
    return tmpBestTree;
}

void Floorplanner::packBlock(TNode* node, LNode* head)
{
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
