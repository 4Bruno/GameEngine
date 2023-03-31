#include "hierarchy_tree.h"

/* STACK */
stack
NewStack(memory_arena * Arena, u32 Capacity)
{
    stack S = {};
    S.Capacity = Capacity;
    S.Items = PushArray(Arena, Capacity, intptr_t);

    return S;
}

intptr_t *
PopStack(stack * S)
{
    intptr_t * Item = 0;
    if (S->Usage)
    {
        Item = S->Items + (--S->Usage);
    }
    return Item;
}
void
PushStack(memory_arena * Arena, stack * S, intptr_t * PushedItem)
{
    if (S->Capacity <= S->Usage)
    {
        u32 NewCapacity = S->Capacity * S->Capacity;
        PushArray(Arena, (NewCapacity - S->Capacity), tree_node *);
        S->Capacity = NewCapacity;
    }

    intptr_t * Item = (S->Items + S->Usage++);
    *Item = *PushedItem;
}
void
PrintStack(stack * S)
{
    for (int i = (S->Usage - 1); i >= 0; --i)
    {
        //Logn("Stack %i with %s", i, (char *)(*(S->Items + i))->VulkanObj);
    }
}


/* QUEUE */
intptr_t *
PopQueue(queue * Q)
{
    intptr_t * Item = 0;

    if (Q->First)
    {
        queue_item * QItem = Q->First;
        queue_item * Next = QItem->next;
        Item = QItem->Item;

        if (Q->First == Q->Last)
        {
            Q->Last = 0;
        }

        Q->First = Next;
        QItem->next = 0;
        QItem->Item = 0;
        Q->Free += 1;
    }

    return Item;
}

void
PushQueue(memory_arena * Arena, queue * Q, intptr_t * Item)
{
    if (Q->Free == 0)
    {
        u32 NewCapacity = Q->Capacity * Q->Capacity;
        PushSize(Arena, sizeof(queue_item) * (NewCapacity - Q->Capacity));
        Q->Free = (NewCapacity - Q->Capacity);
        Q->Capacity = NewCapacity;
    }

    queue_item * QItem = 0;

    for (u32 i = 0; i < Q->Capacity; ++i)
    {
        QItem = Q->Items + i;
        if (!QItem->Item)
        {
            break;
        }
    }

    QItem->next = 0;
    QItem->Item = Item;

    if (!Q->Last)
    {
        Q->First = QItem;
        Q->Last = QItem;
    }
    else
    {
        Q->Last->next = QItem;
        Q->Last = QItem;
    }

    Q->Free -= 1;

}

queue
CreateQueue(memory_arena * Arena, u32 InitialCapacity)
{
    queue Q;
    Q.Capacity = InitialCapacity;
    Q.Free = Q.Capacity;
    Q.First = 0;
    Q.Last = 0;
    u32 SizeQueue = sizeof(queue_item) * Q.Capacity;
    Q.Items = (queue_item *)PushSize(Arena, SizeQueue);
    RtlZeroMemory(Q.Items, SizeQueue);

    return Q;
}


/* HIERARCHY TREE */

PFN_PRINT_NODE(StubPrintNode)
{
}

int
GetNodeDepth(tree_node * Node)
{
    i32 Depth = 0;

    while (Node)
    {
        Node = Node->Parent;
        Depth += 1;
    }

    return Depth;
}

hierarchy_tree *
CreateHierarchyTree(memory_arena * Arena, u32 SizeLimit, 
                    pfn_compare Comparator, 
                    pfn_delete_node OnNodeDelete,
                    pfn_print_node PrintNode)
{
    u32 AvailableSpace = SizeLimit - sizeof(hierarchy_tree);
    hierarchy_tree * Tree = (hierarchy_tree *)PushSize(Arena, SizeLimit);

    u8 * StartAddrArena = (u8 *)(Tree + sizeof(hierarchy_tree));
    InitializeArena(&Tree->Arena, StartAddrArena, AvailableSpace);

    Tree->Comparator = Comparator;
    Tree->OnNodeDelete = OnNodeDelete;
    Tree->PrintNode = PrintNode ? PrintNode : StubPrintNode;

    return Tree;
}

tree_node **
BFSNode(hierarchy_tree * Tree, void * ID)
{
    tree_node ** Node = &Tree->Root;

    u32 ArenaBeginSize = Tree->Arena.CurrentSize;
    queue Q = CreateQueue(&Tree->Arena, 20);

    while (Node && *Node)
    {
        if (Tree->Comparator(*Node, ID) == 0) break;

        Node = &(*Node)->LeftMostChild;

        while (*Node)
        {
            if (Tree->Comparator(*Node, ID) == 0) break;

            PushQueue(&Tree->Arena, &Q, (intptr_t *)Node);

            Node = &(*Node)->RightSibling;
        }; 

        if (*Node) break;

        Node = (tree_node **)PopQueue(&Q);
    }

    Tree->Arena.CurrentSize = ArenaBeginSize;

    return Node;
}

tree_node **
VisitDepth(hierarchy_tree * Tree, tree_node ** Node, void * FindThis, stack * S, b32 Print = false)
{
    while (Node && *Node)
    {
        //Assert((*Node)->VulkanObj);

        if (Tree->Comparator(*Node, FindThis) == 0)
        {
            return Node;
        }

        if (Print)
        {
            Tree->PrintNode(*Node);
        }

        PushStack(&Tree->Arena, S, (intptr_t *)Node);
        Node = &(*Node)->LeftMostChild;
    }

    return Node;
}

tree_node *
HierarchyTreeFind(hierarchy_tree * Tree, void * ID)
{
    tree_node ** Node = BFSNode(Tree, ID);
    tree_node * Result = 0;

    if (Node)
    {
        Result = *Node;
    }

    return Result;
}

void
HierarchyTreeDropBranch(hierarchy_tree * Tree, void * ID, b32 KeepNodeByIDAlive)
{
    tree_node ** Node = BFSNode(Tree, ID);

    if (Node)
    {
        //Logn("Found node for %s (Depth: %i, Children:%s)", (char*)VulkanObj, GetNodeDepth(*Node), (*Node)->LeftMostChild ? "Yes" : "No");

        tree_node ** Child = &(*Node)->LeftMostChild;
        stack S = NewStack(&Tree->Arena, 20);

        VisitDepth(Tree, Child, 0, &S);

        //PrintStack(&S);
        Child = (tree_node **)PopStack(&S);
        while ( Child )
        {
            Assert((*Child)->LeftMostChild == 0);

            //Logn("Freeing resource %s", (char *)(*Child)->VulkanObj);
            Tree->OnNodeDelete((*Child), (*Child)->Parent);
            
            tree_node * RightSibling = (*Child)->RightSibling;
            tree_node ** ParentNewLeftMost = &(*Child)->Parent->LeftMostChild;
            *ParentNewLeftMost = RightSibling;

            (*Child)->RightSibling = Tree->FreeList;
            Tree->FreeList = (*Child);

            if (RightSibling)
            {
                Child = ParentNewLeftMost;
                VisitDepth(Tree, Child, 0, &S);
            }

            Child = (tree_node **)PopStack(&S);
        }

        if (!KeepNodeByIDAlive)
        {
            tree_node * Parent = (*Node)->Parent;
            tree_node * ThisNode = (*Node);
            if (Parent)
            {
                if (Parent->LeftMostChild == ThisNode)
                {
                    Parent->LeftMostChild = ThisNode->RightSibling;
                }
                else
                {
                    Child = &ThisNode->Parent->LeftMostChild;
                    do
                    {
                        if ((*Child)->RightSibling == ThisNode) 
                        {
                            break;
                        }
                        Child = &(*Child)->RightSibling;
                    } while (*Child);
                    Assert(Child && *Child);
                    (*Child)->RightSibling = ThisNode->RightSibling;
                }
            }

            Tree->OnNodeDelete(ThisNode, Parent);

            ThisNode->RightSibling = Tree->FreeList;
            Tree->FreeList = ThisNode;
            if (!Parent) Tree->Root = 0;
        }


    } // node exists
}

void
ShowTree(hierarchy_tree * Tree)
{
    if (Tree->Root == 0) return;

    tree_node ** Node = &Tree->Root;

    u32 ArenaBeginSize = Tree->Arena.CurrentSize;
    queue Q = CreateQueue(&Tree->Arena, 20);

    Tree->PrintNode(*Node);

    while (Node && *Node)
    {
        Node = &(*Node)->LeftMostChild;

        while (*Node)
        {
            Tree->PrintNode(*Node);

            PushQueue(&Tree->Arena, &Q, (intptr_t *)Node);

            Node = &(*Node)->RightSibling;
        }

        Node = (tree_node **)PopQueue(&Q);
    }

    Tree->Arena.CurrentSize = ArenaBeginSize;
}


b32
HierarchyTreeAdd(hierarchy_tree * Tree, void * ParentID, void * Data)
{
    tree_node * Node = 0;
    if (!Tree->FreeList)
    {
        Node = PushStruct(&Tree->Arena, tree_node);
    }
    else
    {
        Node = Tree->FreeList;
        Tree->FreeList = Node->RightSibling;
    }

    tree_node ** ParentNode = 0;

    if (!ParentID)
    {
        Tree->Root = Node;
    }
    else
    {
        ParentNode = BFSNode(Tree, ParentID);

        Assert(ParentNode);

        tree_node ** Child = &(*ParentNode)->LeftMostChild;
        while (*Child && (*Child)->RightSibling)
        {
            Child = &((*Child)->RightSibling);
        }
        if (*Child)
        {
            (*Child)->RightSibling = Node;
        }
        else
        {
            (*ParentNode)->LeftMostChild = Node;
        }
    }

    Node->Parent = ParentNode ? *ParentNode : 0;
    Node->RightSibling = 0;
    Node->LeftMostChild = 0;
    Node->Data = Data;

    return 1;
}
