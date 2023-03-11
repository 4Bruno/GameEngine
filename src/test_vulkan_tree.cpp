#include "preprocessor_assets.h"

struct vulkan_node
{
    void * VulkanObj;
    vulkan_node * Parent;
    vulkan_node * LeftMostChild; 
    vulkan_node * RightSibling;
};

struct vulkan_tree
{
    memory_arena Arena;
    vulkan_node * Root;
};

int
GetNodeDepth(vulkan_node * Node)
{
    i32 Depth = 0;

    while (Node)
    {
        Node = Node->Parent;
        Depth += 1;
    }

    return Depth;
}

vulkan_tree *
CreateVulkanTree(memory_arena * Arena)
{
    u32 SizeArena = Megabytes(2);
    u32 SizeRequirements = sizeof(vulkan_tree) + SizeArena;

    vulkan_tree * Tree = (vulkan_tree *)PushSize(Arena, SizeRequirements);

    u8 * StartAddrArena = (u8 *)(Tree + sizeof(vulkan_tree));
    InitializeArena(&Tree->Arena, StartAddrArena, SizeArena);

    return Tree;
}

struct queue_item
{
    vulkan_node * Node;
    queue_item * next;
};

struct stack
{
    u32 Capacity;
    u32 Usage;
    vulkan_node * Items;
};

stack
NewStack(memory_arena * Arena, u32 Capacity)
{
    stack S = {};
    S.Capacity = Capacity;
    S.Items = PushArray(Arena, Capacity, vulkan_node);

    return S;
}

vulkan_node *
PopStack(stack * S)
{
    vulkan_node * Node = 0;
    if (S->Usage)
    {
        Node = S->Items + (--S->Usage);
    }
    return Node;
}
void
PushStack(memory_arena * Arena, stack * S, vulkan_node * Node)
{
    if (S->Capacity <= S->Usage)
    {
        u32 NewCapacity = S->Capacity * S->Capacity;
        PushArray(Arena, (NewCapacity - S->Capacity), vulkan_node);
        S->Capacity = NewCapacity;
    }

    vulkan_node * Item = S->Items + S->Usage++;
    *Item = *Node;
}

struct queue
{
    u32 Capacity;
    u32 Free;
    queue_item * First;
    queue_item * Last;
    queue_item * Items;
};

vulkan_node *
PopQueue(queue * Q)
{
    vulkan_node * Node = 0;
    if (Q->First)
    {
        queue_item * Item = Q->First;
        queue_item * Next = 0;

        if (Q->First == Q->Last)
        {
            Q->Last = 0;
        }

        if (Item) 
        {
            Next = Item->next;
            Node = Item->Node;
        }

        Q->First = Next;
        Item->next = 0;
        Item->Node = 0;
        Q->Free += 1;
    }


    return Node;
}

void
PushQueue(memory_arena * Arena, queue * Q, vulkan_node * Node)
{
    if (Q->Free == 0)
    {
        PushSize(Arena, sizeof(queue_item) * Q->Capacity);
        Q->Free = Q->Capacity;
        Q->Capacity *= 2;
    }

    queue_item * Item = 0;

    for (u32 i = 0; i < Q->Capacity; ++i)
    {
        Item = Q->Items + i;
        if (!Item->Node)
        {
            break;
        }
    }

    Item->next = 0;
    Item->Node = Node;

    if (!Q->Last)
    {
        Q->First = Item;
        Q->Last = Item;
    }
    else
    {
        Q->Last->next = Item;
        Q->Last = Item;
    }

    Q->Free -= 1;

}

vulkan_node *
VisitLevel(vulkan_tree * Tree, vulkan_node * Node, void * FindThis, queue * Q, b32 Print = false)
{
    while (Node)
    {
        Assert(Node->VulkanObj);
        if (Node->VulkanObj == FindThis)
        {
            return Node;
        }

        if (Print)
        {
            Logn("%*cNode %s",GetNodeDepth(Node) * 4, ' ',(char *)Node->VulkanObj);
        }

        PushQueue(&Tree->Arena, Q, Node);
        Node = Node->RightSibling;
    }

    return 0;
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

vulkan_node * 
VisitDepth(vulkan_tree * Tree, vulkan_node * Node, void * FindThis, stack * S, b32 Print = false)
{
    while (Node)
    {
        Assert(Node->VulkanObj);
        if (Node->VulkanObj == FindThis)
        {
            return Node;
        }

        if (Print)
        {
            Logn("Child %p", Node->VulkanObj);
        }

        PushStack(&Tree->Arena, S, Node);
        Node = Node->LeftMostChild;
    }

    return Node;
}

void
PrintStack(stack * S)
{
    for (int i = (S->Usage - 1); i >= 0; --i)
    {
        Logn("Stack %i with %p", i, (S->Items + i)->VulkanObj);
    }
}

vulkan_node *
BFSNode(vulkan_tree * Tree, void * VulkanObj)
{
    vulkan_node * Node = Tree->Root;

    u32 ArenaBeginSize = Tree->Arena.CurrentSize;
    queue Q = CreateQueue(&Tree->Arena, 20);

    while (Node)
    {
        if (Node->VulkanObj != VulkanObj)
        {
            Node = VisitLevel(Tree, Node->LeftMostChild, VulkanObj, &Q);
        }
        if (Node)
        {
            break;
        }
        Node = PopQueue(&Q);
    }

    Tree->Arena.CurrentSize = ArenaBeginSize;

    return Node;
}

void
VulkanRemoveObject(vulkan_tree * Tree, void * VulkanObj)
{

    vulkan_node * Node = BFSNode(Tree, VulkanObj);

    if (Node)
    {
        Logn("Found node for %p (Depth: %i, Children:%s)", VulkanObj, GetNodeDepth(Node), Node->LeftMostChild ? "Yes" : "No");

        vulkan_node * Child = Node->LeftMostChild;
        stack S = NewStack(&Tree->Arena, 20);

        VisitDepth(Tree, Child, 0, &S);
        //PrintStack(&S);
        while ( (Child = PopStack(&S)) )
        {
            Assert(Child->LeftMostChild == 0);
            Logn("Freeing resource %p", Child->VulkanObj);
            Child->Parent->LeftMostChild = 0;
            if (Child->RightSibling)
            {
                Child = Child->RightSibling;
                VisitDepth(Tree, Child, 0, &S);
            }
        }
    } // node exists
}


b32
VulkanAddObject(vulkan_tree * Tree, void * ParentObj, void * VulkanObj)
{
    vulkan_node * Node = PushStruct(&Tree->Arena, vulkan_node);
    vulkan_node * ParentNode = 0;

    if (!ParentObj)
    {
        Tree->Root = Node;
    }
    else
    {
        ParentNode = BFSNode(Tree, ParentObj);

        Assert(ParentNode);

        vulkan_node ** Child = &ParentNode->LeftMostChild;
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
            ParentNode->LeftMostChild = Node;
        }
    }

    Node->Parent = ParentNode;
    Node->RightSibling = 0;
    Node->LeftMostChild = 0;
    Node->VulkanObj = VulkanObj;

    return 1;
}

void
ShowTree(vulkan_tree * Tree)
{
    u32 ArenaBeginSize = Tree->Arena.CurrentSize;

    queue Q = CreateQueue(&Tree->Arena, 20);
    vulkan_node * ParentNode = Tree->Root;
    while (ParentNode)
    {
        Logn("%*cParent %s",GetNodeDepth(ParentNode) * 4, ' ',(char *)ParentNode->VulkanObj);
        ParentNode = VisitLevel(Tree, ParentNode->LeftMostChild, 0, &Q, true);
        //Logn("Queue Free %i", Q.Free);
        if (ParentNode)
        {
            break;
        }
        ParentNode = PopQueue(&Q);
    }
    Logn("Queue Free %i", Q.Free);

    Tree->Arena.CurrentSize = ArenaBeginSize;
}

int main()
{
    memory_arena Arena = {};
    Arena.Base = (u8 *)malloc(Megabytes(3));
    Arena.MaxSize = Megabytes(3);
    Arena.CurrentSize = 0;

    vulkan_tree * Tree = CreateVulkanTree(&Arena);

    char Instance[20] = "Instance";
    char Device[20] = "Device";
    char FrameBuffer[20] = "FrameBuffer";
    char VertexBuffer[20] = "VertexBuffer";
    char IndexBuffer[20] = "IndexBuffer";

    VulkanAddObject(Tree, 0, &Instance);
    VulkanAddObject(Tree, &Instance, &Device);
    VulkanAddObject(Tree, &Device, &FrameBuffer);
    VulkanAddObject(Tree, &Device, &VertexBuffer);
    VulkanAddObject(Tree, &Device, &IndexBuffer);

    char FrameData01[20] = "FrameData01";
    char FrameData02[20] = "FrameData02";
    VulkanAddObject(Tree, &FrameBuffer, &FrameData01);
    VulkanAddObject(Tree, &FrameBuffer, &FrameData02);

    ShowTree(Tree);

    //VulkanRemoveObject(Tree, &FrameBuffer);
    VulkanRemoveObject(Tree, &Device);


}
