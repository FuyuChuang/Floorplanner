/****************************************************************************
  FileName  [ bStarTree.cpp ]
  Synopsis  [ Implementation of the B*-tree. ]
  Author    [ Fu-Yu Chuang ]
  Date      [ 2017.4.25 ]
****************************************************************************/
#include <iostream>
#include "bStarTree.h"

// constructor and destructor
BStarTree::BStarTree(vector<Block*> blockList)
{
    // initial solution is a line...
    _nodeList.push_back(new TNode(0));
    _root = _nodeList[0];
    for (size_t i = 1, end = blockList.size(); i < end; ++i) {
        _nodeList.push_back(new TNode(i));
        _nodeList[i]->_parent = _nodeList[i-1];
        _nodeList[i-1]->_right = _nodeList[i];
    }
}

BStarTree::BStarTree(const BStarTree& tree)
{
    _nodeList.push_back(new TNode(tree._root->_id, tree._root->_orient));
    _root = _nodeList[0];
    this->copyTree(&(_root->_left), tree._root->_left, _root);
    this->copyTree(&(_root->_right), tree._root->_right, _root);
}

BStarTree& BStarTree::operator = (const BStarTree& tree)
{
    this->clear();
    _nodeList.push_back(new TNode(tree._root->_id, tree._root->_orient));
    _root = _nodeList[0];
    this->copyTree(&(_root->_left), tree._root->_left, _root);
    this->copyTree(&(_root->_right), tree._root->_right, _root);
}

BStarTree::~BStarTree()
{
    this->clear();
}

// member functions
vector<BStarTree> BStarTree::perturb()
{
    vector<BStarTree> trees;
    srand(time(NULL));
    if (rand() % 3 == 0) {
        this->rotate(trees);
    }
    else if (rand() % 3 == 1) {
        this->swap(trees);
    }
    else {
        this->delAndInsert(trees);
    }
    return trees;
}

void BStarTree::pack()
{

    return;
}

// private member functions
void BStarTree::copyTree(TNode** nodePtr, const TNode* cNode, TNode* prev)
{
    if (cNode != NULL) {
        _nodeList.push_back(new TNode(cNode->_id, cNode->_orient));
        *nodePtr = _nodeList.back();
        _nodeList.back()->_parent = prev;
        this->copyTree(&(_nodeList.back()->_left), cNode->_left, _nodeList.back());
        this->copyTree(&(_nodeList.back()->_right), cNode->_right, _nodeList.back());
    }
    return;
}

void BStarTree::clear()
{
    for (size_t i = 0, end = _nodeList.size(); i < end; ++i) {
        delete _nodeList[i];
    }
    for (size_t i = 0, end = _contourList.size(); i < end; ++i) {
        delete _contourList[i];
    }
    return;
}

void BStarTree::rotate(vector<BStarTree>& trees)
{
    trees.push_back(*(this));
    int id = rand() % _nodeList.size();
    trees.back().rotateNode(id);
    return;
}

void BStarTree::swap(vector<BStarTree>& trees)
{
    int id1 = 0, id2 = 0;
    while (id1 == id2) {
        id1 = rand() % _nodeList.size();
        id2 = rand() % _nodeList.size();
    }
    for (size_t i = 0; i < 2; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            trees.push_back(*(this));
            if (i == 1)
                trees.back().rotateNode(id1);
            if (j == 1)
                trees.back().rotateNode(id2);
            trees.back().swapNodes(id1, id2);
        }
    }
    return;
}

void BStarTree::delAndInsert(vector<BStarTree>& trees)
{
    int id1 = 0, id2 = 0;
    while(id1 == id2) {
        id1 = rand() % _nodeList.size();
        id2 = rand() % _nodeList.size();
    }
    for (size_t i = 0; i < 2; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            for (size_t k = 0; k < 2; ++k) {
                trees.push_back(*(this));
                if (i == 1)
                    trees.back().rotateNode(id1);
                trees.back().deleteNode(id1);
                trees.back().insertNode(id1, id2, j, k);
            }
        }
    }
    return;
}

void BStarTree::swapNodes(int id1, int id2)
{

    return;
}

void BStarTree::rotateNode(int id)
{

    return;
}

void BStarTree::deleteNode(int id)
{

    return;
}

void BStarTree::insertNode(int id1, int id2, bool p_right, bool n_right)
{

    return;
}
