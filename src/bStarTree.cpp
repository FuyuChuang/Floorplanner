/****************************************************************************
  FileName  [ bStarTree.cpp ]
  Synopsis  [ Implementation of the B*-tree. ]
  Author    [ Fu-Yu Chuang ]
  Date      [ 2017.4.27 ]
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
    _nodeList.push_back(new TNode(0));
    _root = _nodeList[0];
    for (size_t i = 1, end = blockList.size(); i < end; ++i) {
        _nodeList.push_back(new TNode(i));
        /*
        _nodeList[i]->_parent = _nodeList[i-1];
        _nodeList[i-1]->_right = _nodeList[i];
        */
        if (i % 2 == 0) {
            _nodeList[i]->_parent = _nodeList[i/2-1];
            assert(_nodeList[i/2-1]->_right == NULL);
            _nodeList[i/2-1]->_right = _nodeList[i];
        }
        else {
            _nodeList[i]->_parent = _nodeList[i/2];
            assert(_nodeList[i/2]->_left == NULL);
            _nodeList[i/2]->_left = _nodeList[i];
        }
    }
}

BStarTree::BStarTree(const BStarTree& tree)
{
    this->clear();
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
    size_t r = rand();
    if (r % 5 == 0) {
        this->rotate(trees);
        // cout << "rotate" << endl;
    }
    else if (r % 5 <= 2) {
        this->swap(trees);
        // cout << "swap" << endl;
    }
    else {
        this->delAndInsert(trees);
        // cout << "del" << endl;
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
    // cout << "Rotate " << id << endl;
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
    // cout << "Swap " << id1 << " & " << id2 << endl;
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
    // cout << "Delete " << id1 << " and insert to " << id2 << endl;
    return;
}

void BStarTree::swapNodes(int id1, int id2)
{
    TNode* node1 = _nodeList[id1];
    TNode* node2 = _nodeList[id2];
    int b_id1 = node1->getId();
    int b_id2 = node2->getId();
    bool orient1 = node1->getOrient();
    node1->setOrient(node2->getOrient());
    node1->setId(b_id1);
    node2->setOrient(orient1);
    node2->setId(b_id2);

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
                assert(p != NULL);
                assert(l != NULL);
                assert(r != NULL);
                node->_parent = l;
                node->_left = l->_left;
                node->_right = l->_right;
                if (l->_left != NULL)
                    l->_left->_parent = node;
                if (l->_right != NULL)
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
                if (l->_left != NULL)
                    l->_left->_parent = node;
                if (l->_right != NULL)
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
