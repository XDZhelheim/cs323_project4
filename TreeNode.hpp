#ifndef TREENODE_HPP
#define TREENODE_HPP

#include <vector>
#include <initializer_list>
#include <string>
#include <fstream>
#include <unistd.h>

FILE *output_file;
int has_error;

using std::endl;
using std::initializer_list;
using std::ofstream;
using std::string;
using std::vector;

enum DataType
{
    INT_TYPE,
    FLOAT_TYPE,
    CHAR_TYPE,
    ID_TYPE,
    TYPE_TYPE,
    CHILD,
    OTHER
};

struct TreeNode
{
    string name;
    DataType type;
    int pos;
    TreeNode *parent = nullptr;
    string data;
    vector<TreeNode *> child;
};

struct TreeNode *root;

TreeNode *create_node(string name, int position = 0, DataType type = DataType::OTHER, string val = "")
{
    TreeNode *node = new TreeNode;
    node->name = name;
    node->type = type;
    node->pos = position;
    node->data = val;
    return node;
}

TreeNode *create_child_node(string name, int position, initializer_list<TreeNode *> child)
{
    TreeNode *node = new TreeNode;
    node->name = name;
    node->type = DataType::CHILD;
    node->pos = position;
    for (auto c : child)
    {
        c->parent = node;
        node->child.push_back(c);
    }
    return node;
}

#endif