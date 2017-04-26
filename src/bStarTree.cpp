/****************************************************************************
  FileName  [ bStarTree.cpp ]
  Synopsis  [ Implementation of the B*-tree. ]
  Author    [ Fu-Yu Chuang ]
  Date      [ 2017.4.25 ]
****************************************************************************/
#include <iostream>
#include <cassert>
#include "bStarTree.h"

// constructor and destructor
BStarTree::BStarTree()
{
    _root = NULL;
}
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
        // cout << "rotate" << endl;
        this->rotate(trees);
    }
    else if (rand() % 3 == 1) {
        // cout << "swap" << endl;
        this->swap(trees);
    }
    else {
        // cout << "del" << endl;
        this->delAndInsert(trees);
    }
    return trees;
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
    _nodeList.clear();
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
    while (id1 == id2) {
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
    TNode* node1 = _nodeList[id1];
    TNode* node2 = _nodeList[id2];

    if (node1 == _root)
        _root = node2;
    else if (node2 == _root)
        _root = node1;

    TNode* p1 = node1->_parent;
    TNode* p2 = node2->_parent;
    TNode* l1 = node1->_left;
    TNode* l2 = node2->_left;
    TNode* r1 = node1->_right;
    TNode* r2 = node2->_right;

    if (node1->_parent != node2 && node2->_parent != node1) {
        node1->_parent = p2;
        node2->_parent = p1;
        node1->_left = l2;
        node2->_left = l1;
        node1->_right = r2;
        node2->_right = r1;
        if (p1 != NULL) {
            if (p1->_left == node1)
                p1->_left = node2;
            else
                p1->_right = node2;
        }
        if (p2 != NULL) {
            if (p2->_left == node2)
                p2->_left = node1;
            else
                p2->_right = node1;
        }
        if (l1 != NULL)
            l1->_parent = node2;
        if (r1 != NULL)
            r1->_parent = node2;
        if (l2 != NULL)
            l2->_parent = node1;
        if (r2 != NULL)
            r2->_parent = node1;
    }
    else if (node1->_parent == node2) {
        node1->_parent = p2;
        node2->_parent = node1;
        if (node2->_left == node1) {
            node1->_left = node2;
            node1->_right = r2;
            if (r2 != NULL)
                r2->_parent = node1;
        }
        else {
            node1->_right = node2;
            node1->_left = l2;
            if (l2 != NULL)
                l2->_parent = node1;
        }
        node2->_left = l1;
        node2->_right = r1;
        if (p2 != NULL) {
            if (p2->_left == node2)
                p2->_left = node1;
            else
                p2->_right = node1;
        }
        if (l1 != NULL)
            l1->_parent = node2;
        if (r1 != NULL)
            r1->_parent = node2;
    }
    else if (node2->_parent == node1) {
        node2->_parent = p1;
        node1->_parent = node2;
        if (node1->_left == node2) {
            node2->_left = node1;
            node2->_right = r1;
            if (r1 != NULL)
                r1->_parent = node2;
        }
        else {
            node2->_right = node1;
            node2->_left = l1;
            if (l1 != NULL)
                l1->_parent = node1;
        }
        node1->_left = l2;
        node1->_right = r2;
        if (p1 != NULL) {
            if (p1->_left == node1)
                p1->_left = node2;
            else
                p1->_right = node2;
        }
        if (l2 != NULL)
            l2->_parent = node1;
        if (r2 != NULL)
            r2->_parent = node1;
    }

    return;
}

void BStarTree::rotateNode(int id)
{
    _nodeList[id]->rotate();
    return;
}

void BStarTree::deleteNode(int id)
{
    TNode* node = _nodeList[id];
    while (true) {
        if (node != _root) {
            if (node->_left == NULL && node->_right == NULL) {
                if (node->_parent->_left == node) {
                    node->_parent->_left = NULL;
                }
                else if (node->_parent->_right == node) {
                    node->_parent->_right = NULL;
                }
                else {
                    cerr << "Error 1" << endl;
                }
                node->_parent = NULL;
                break;
            }
            else if (node->_left == NULL) {
                node->_right->_parent = node->_parent;
                if (node->_parent->_left == node) {
                    node->_parent->_left = node->_right;
                }
                else if (node->_parent->_right == node) {
                    node->_parent->_right = node->_right;
                }
                else {
                    cerr << "Error 2" << endl;
                }
                node->_parent = NULL;
                node->_right = NULL;
                break;
            }
            else if (node->_right == NULL) {
                node->_left->_parent = node->_parent;
                if (node->_parent->_left == node) {
                    node->_parent->_left = node->_left;
                }
                else if (node->_parent->_right == node) {
                    node->_parent->_right = node->_left;
                }
                else {
                    cerr << "Error 3" << endl;
                }
                node->_parent = NULL;
                node->_left = NULL;
                break;
            }
            else {
                TNode* p = node->_parent;
                TNode* l = node->_left;
                TNode* r = node->_right;
                node->_parent = l;
                node->_left = l->_left;
                node->_right = l->_right;
                l->_left->_parent = node;
                l->_right->_parent = node;
                l->_left = node;
                l->_right = r;
                l->_parent = p;
                r->_parent = l;
                if (p->_left == node) {
                    p->_left = l;
                }
                else if (p->_right == node) {
                    p->_right = l;
                }
                else {
                    cerr << "Error 4" << endl;
                }
            }
        }
        else {
            // cout << "Delete root!" << endl;
            if (node->_left == NULL && node->_right == NULL) {
                cout << "Not possible" << endl;
                break;
            }
            else if (node->_left == NULL) {
                node->_right->_parent = node->_parent;
                assert(node->_right->_parent == NULL);
                _root = node->_right;
                node->_right = NULL;
                break;
            }
            else if (node->_right == NULL) {
                node->_left->_parent = node->_parent;
                assert(node->_left->_parent == NULL);
                _root = node->_left;
                node->_left = NULL;
                break;
            }
            else {
                TNode* p = node->_parent;
                TNode* l = node->_left;
                TNode* r = node->_right;
                node->_parent = l;
                node->_left = l->_left;
                node->_right = l->_right;
                l->_left->_parent = node;
                l->_right->_parent = node;
                l->_left = node;
                l->_right = r;
                l->_parent = p;
                r->_parent = l;
                assert(p == NULL);
                _root = l;
            }
        }
    }
    return;
}

// Insert node(id1) as node(id2)'s left/right child and node(id2)'s left/right
// child becomes node(id1)'s left/right child
// Note that insertNode should be called only when node(id1) has been deleted
void BStarTree::insertNode(int id1, int id2, bool p_right, bool n_right)
{
    TNode* node1 = _nodeList[id1];
    TNode* node2 = _nodeList[id2];
    assert(node1->_parent == NULL && node1->_left == NULL && node1->_right == NULL);

    if (p_right) {
        TNode* r = node2->_right;
        node2->_right = node1;
        node1->_parent = node2;
        if (n_right)
            node1->_right = r;
        else
            node1->_left = r;
        if (r != NULL)
            r->_parent = node1;
    }
    else {
        TNode* l = node2->_left;
        node2->_left = node1;
        node1->_parent = node2;
        if (n_right)
            node1->_right = l;
        else
            node1->_left = l;
        if (l != NULL)
            l->_parent = node1;
    }
    return;
}
