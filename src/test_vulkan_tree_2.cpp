#include "hierarchy_tree.h"

PFN_PRINT_NODE(VulkanPrintNode)
{
    Logn("%*cParent %s",GetNodeDepth(Node) * 4, ' ',(char *)(Node)->Data);
}

PFN_COMPARE(VulkanCompareObject)
{
    i32 Result = -1;

    tree_node * Node = (tree_node *)A;

    if (Node->Data == B)
    {
        Result = 0;
    }

    return Result;
}

int main()
{
    memory_arena Arena = {};
    Arena.Base = (u8 *)malloc(Megabytes(3));
    Arena.MaxSize = Megabytes(3);
    Arena.CurrentSize = 0;

    hierarchy_tree * Tree = 
        CreateHierarchyTree(&Arena, Megabytes(3), VulkanCompareObject, VulkanPrintNode);

    char Instance[20] = "Instance";
    char Device[20] = "Device";
    char FrameBuffer[20] = "FrameBuffer";
    char VertexBuffer[20] = "VertexBuffer";
    char IndexBuffer[20] = "IndexBuffer";

    HierarchyTreeAdd(Tree, 0, &Instance);
    HierarchyTreeAdd(Tree, &Instance, &Device);
    HierarchyTreeAdd(Tree, &Device, &FrameBuffer);
    HierarchyTreeAdd(Tree, &Device, &VertexBuffer);
    HierarchyTreeAdd(Tree, &Device, &IndexBuffer);

    //ShowTree(Tree);

    char FrameData01[20] = "FrameData01";
    char FrameData02[20] = "FrameData02";
    HierarchyTreeAdd(Tree, &FrameBuffer, &FrameData01);
    HierarchyTreeAdd(Tree, &FrameBuffer, &FrameData02);

    //ShowTree(Tree);

#if 1
    HierarchyTreeDropBranch(Tree, &FrameBuffer);
    //HierarchyTreeDropBranch(Tree, &Device);
    ShowTree(Tree);
#else
    HierarchyTreeDropBranch(Tree, &VertexBuffer);
    ShowTree(Tree);
    HierarchyTreeDropBranch(Tree, &IndexBuffer);
    ShowTree(Tree);
    HierarchyTreeDropBranch(Tree, &Instance);
#endif

    Logn("PROGRAM RUNS TO END");

}
