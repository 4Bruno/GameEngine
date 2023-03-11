#ifndef HIERARCHY_TREE_H
#define HIERARCHY_TREE_H

#include "game_platform.h"
#include "game_memory.h"

struct tree_node
{
    void * Data;
    tree_node * Parent;
    tree_node * LeftMostChild; 
    tree_node * RightSibling;
};

#define PFN_COMPARE(name) i32 name(void * A, void * B)
typedef PFN_COMPARE(pfn_compare);

#define PFN_PRINT_NODE(name) void name(tree_node * Node)
typedef PFN_PRINT_NODE(pfn_print_node);

#define PFN_DELETE_NODE(name) void name(tree_node * Node, const tree_node * ParentNode)
typedef PFN_DELETE_NODE(pfn_delete_node);

struct hierarchy_tree
{
    memory_arena Arena;
    pfn_compare * Comparator;
    pfn_delete_node * OnNodeDelete;
    pfn_print_node * PrintNode;
    tree_node * Root;
    tree_node * FreeList;
};

struct queue_item
{
    intptr_t * Item;
    queue_item * next;
};

struct queue
{
    u32 Capacity;
    u32 Free;
    queue_item * First;
    queue_item * Last;
    queue_item * Items;
};


struct stack
{
    u32 Capacity;
    u32 Usage;
    intptr_t * Items;
};

void HierarchyTreeDropBranch(hierarchy_tree * Tree,void * ID, b32 KeepNodeByIDAlive = false);
void ShowTree(hierarchy_tree * Tree);
b32 HierarchyTreeAdd(hierarchy_tree * Tree,void * ParentID,void * Data);
hierarchy_tree * CreateHierarchyTree(memory_arena * Arena, 
                                     u32 SizeLimit, 
                                     pfn_compare Comparator, 
                                     pfn_delete_node OnNodeDelete,
                                     pfn_print_node PrintNode = 0);
int GetNodeDepth(tree_node * Node);

tree_node *
HierarchyTreeFind(hierarchy_tree * Tree, void * ID);
#endif
