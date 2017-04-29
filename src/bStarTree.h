/****************************************************************************
  FileName  [ bStarTree.h ]
  Synopsis  [ Define the B*-tree data structure. ]
  Author    [ Fu-Yu Chuang ]
  Date      [ 2017.4.26 ]
****************************************************************************/
#ifndef BSTARTREE_H
#define BSTARTREE_H

#include <vector>
#include <string>
#include "module.h"
using namespace std;

// B*-tree node
class TNode
{
    friend class BStarTree;
    friend class Floorplanner;

public:
    // constructor and destructor
    TNode(size_t id, bool orient = false, TNode* p = NULL, TNode* l = NULL, TNode* r = NULL) :
        _id(id), _orient(orient), _parent(p), _left(l), _right(r) { }
    ~TNode()    { }

    // basic access methods
    size_t getId()      { return _id; }
    bool   getOrient()  { return _orient; }

    // modify methods
    void rotate()               { _orient = !_orient; }
    void setId(size_t id)       { _id = id; }
    void setOrient(bool orient) { _orient = orient; }

private:
    size_t      _id;        // id of the block storing in this node
    bool        _orient;    // record the orientation of the block (0: origin, 1: rotated)
    TNode*      _parent;    // parent of the node
    TNode*      _left;      // left child of the node
    TNode*      _right;     // right child of the node
};


class BStarTree
{
    friend class Floorplanner;

public:
    // constructor and destructor
    BStarTree();
    BStarTree(vector<Block*> blockList);
    BStarTree(const BStarTree& tree);
    BStarTree& operator = (const BStarTree& tree);
    ~BStarTree();

    // perturbing the B*-tree
    vector<BStarTree> perturb();

private:
    TNode*          _root;          // root of the B*-tree
    vector<TNode*>  _nodeList;      // list of nodes in the tree

    // private member functions
    void copyTree(TNode** nodePtr, const TNode* cNode, TNode* prev);
    void clear();

    // manipulating the B*-tree to get the "neighborhood structures"
    void rotate(vector<BStarTree>& trees);
    void swap(vector<BStarTree>& trees);
    void delAndInsert(vector<BStarTree>& trees);

    // manipulating the nodes
    void swapNodes(int id1, int id2);
    void rotateNode(int id);
    void deleteNode(int id);
    void insertNode(int id1, int id2, bool p_right, bool n_right);
};

#endif  // BSTARTREE_H
