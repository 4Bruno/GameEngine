#include "game_platform.h"
#include "noise_perlin_2.cpp"
#include "game_math.h"
#include "game.h"
#include "game_memory.h"

/*
 * In a cube each edge represented by a bit is a 8bit (1 byte)
 * 0x0000 0000
 * There is 255 combinations
 * We check density of each edge and can be (-,0,+)
 * - meaning outside of terrain : set bit 0
 * 0 meaning surface of terrain
 * + meaning inside of terrain  : set bit 1
 * IF:
 *  * All edges are - then no polygons as we are outside of terrain. This is case were all bits are 0.
 *  * All edges are + then no polygons as we are inside of terrain. This is case were all bits are 255.
 *  * Any other case falls within [1,254]
 *  We use below lookup table to check for each of the 254 cases how many polygons will be required.
 *  Note there is a max number of polygons of 5 that will be required in the worst case
 *
 *  Front face vertices index
 *  1 2
 *  0 3
 *
 *  Back face vertices index
 *  5 6
 *  4 7
 */
i32 LookupNumPolygon[256] =
{ 
    0, 1, 1, 2, 1, 2, 2, 3,  1, 2, 2, 3, 2, 3, 3, 2,  1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 3,  
    1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 3,  2, 3, 3, 2, 3, 4, 4, 3,  3, 4, 4, 3, 4, 5, 5, 2,  
    1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 3,  2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 4, 5, 5, 4,  
    2, 3, 3, 4, 3, 4, 2, 3,  3, 4, 4, 5, 4, 5, 3, 2,  3, 4, 4, 3, 4, 5, 3, 2,  4, 5, 5, 4, 5, 2, 4, 1,  
    1, 2, 2, 3, 2, 3, 3, 4,  2, 3, 3, 4, 3, 4, 4, 3,  2, 3, 3, 4, 3, 4, 4, 5,  3, 2, 4, 3, 4, 3, 5, 2,  
    2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 4, 5, 5, 4,  3, 4, 4, 3, 4, 5, 5, 4,  4, 3, 5, 2, 5, 4, 2, 1,  
    2, 3, 3, 4, 3, 4, 4, 5,  3, 4, 4, 5, 2, 3, 3, 2,  3, 4, 4, 5, 4, 5, 5, 2,  4, 3, 5, 4, 3, 2, 4, 1,  
    3, 4, 4, 5, 4, 5, 3, 4,  4, 5, 5, 2, 3, 4, 2, 1,  2, 3, 3, 2, 3, 4, 2, 1,  3, 2, 4, 1, 2, 1, 1, 0
};

/*
 * Once we know how many polygons are needed, we need to know which edges will be used to create
 * each triangle. We need 3 edges to form a triangle.
 * Below lookup table tells us:
 *  - for each 256 cases
 *  - for each of the 5 posible triangles
 *  - the 3 edges that are required to check
 * Note -1 stands for "No edge case here".
 * This is the distribution of edges in the cube with the int associated
 *
 * Front face edges
 *   1
 * 0   2
 *   3
 *
 * Back face edges
 *   5
 * 4   6
 *   7
 *
 * front-back edges
 * 9   10
 * 
 * 8   11
 *
 * Now you can see that there are 12 edges [0,11]
 * This can be represented using 4bit 0000
 */

i32 LookupPolygonEdges[256][5][3] =
{
    {{ -1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  8,  3} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  1,  9} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  8,  3} , { 9,  8,  1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  2, 10} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  8,  3} , { 1,  2, 10} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9,  2, 10} , { 0,  2,  9} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  2,  8,  3} , { 2, 10,  8} , {10,  9,  8} , {-1, -1, -1} , {-1, -1, -1} },
    {{  3, 11,  2} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0, 11,  2} , { 8, 11,  0} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  9,  0} , { 2,  3, 11} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1, 11,  2} , { 1,  9, 11} , { 9,  8, 11} , {-1, -1, -1} , {-1, -1, -1} },
    {{  3, 10,  1} , {11, 10,  3} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0, 10,  1} , { 0,  8, 10} , { 8, 11, 10} , {-1, -1, -1} , {-1, -1, -1} },
    {{  3,  9,  0} , { 3, 11,  9} , {11, 10,  9} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9,  8, 10} , {10,  8, 11} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  4,  7,  8} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  4,  3,  0} , { 7,  3,  4} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  1,  9} , { 8,  4,  7} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  4,  1,  9} , { 4,  7,  1} , { 7,  3,  1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  2, 10} , { 8,  4,  7} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  3,  4,  7} , { 3,  0,  4} , { 1,  2, 10} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9,  2, 10} , { 9,  0,  2} , { 8,  4,  7} , {-1, -1, -1} , {-1, -1, -1} },
    {{  2, 10,  9} , { 2,  9,  7} , { 2,  7,  3} , { 7,  9,  4} , {-1, -1, -1} },
    {{  8,  4,  7} , { 3, 11,  2} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{ 11,  4,  7} , {11,  2,  4} , { 2,  0,  4} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9,  0,  1} , { 8,  4,  7} , { 2,  3, 11} , {-1, -1, -1} , {-1, -1, -1} },
    {{  4,  7, 11} , { 9,  4, 11} , { 9, 11,  2} , { 9,  2,  1} , {-1, -1, -1} },
    {{  3, 10,  1} , { 3, 11, 10} , { 7,  8,  4} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1, 11, 10} , { 1,  4, 11} , { 1,  0,  4} , { 7, 11,  4} , {-1, -1, -1} },
    {{  4,  7,  8} , { 9,  0, 11} , { 9, 11, 10} , {11,  0,  3} , {-1, -1, -1} },
    {{  4,  7, 11} , { 4, 11,  9} , { 9, 11, 10} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9,  5,  4} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9,  5,  4} , { 0,  8,  3} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  5,  4} , { 1,  5,  0} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  8,  5,  4} , { 8,  3,  5} , { 3,  1,  5} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  2, 10} , { 9,  5,  4} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  3,  0,  8} , { 1,  2, 10} , { 4,  9,  5} , {-1, -1, -1} , {-1, -1, -1} },
    {{  5,  2, 10} , { 5,  4,  2} , { 4,  0,  2} , {-1, -1, -1} , {-1, -1, -1} },
    {{  2, 10,  5} , { 3,  2,  5} , { 3,  5,  4} , { 3,  4,  8} , {-1, -1, -1} },
    {{  9,  5,  4} , { 2,  3, 11} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0, 11,  2} , { 0,  8, 11} , { 4,  9,  5} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  5,  4} , { 0,  1,  5} , { 2,  3, 11} , {-1, -1, -1} , {-1, -1, -1} },
    {{  2,  1,  5} , { 2,  5,  8} , { 2,  8, 11} , { 4,  8,  5} , {-1, -1, -1} },
    {{ 10,  3, 11} , {10,  1,  3} , { 9,  5,  4} , {-1, -1, -1} , {-1, -1, -1} },
    {{  4,  9,  5} , { 0,  8,  1} , { 8, 10,  1} , { 8, 11, 10} , {-1, -1, -1} },
    {{  5,  4,  0} , { 5,  0, 11} , { 5, 11, 10} , {11,  0,  3} , {-1, -1, -1} },
    {{  5,  4,  8} , { 5,  8, 10} , {10,  8, 11} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9,  7,  8} , { 5,  7,  9} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9,  3,  0} , { 9,  5,  3} , { 5,  7,  3} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  7,  8} , { 0,  1,  7} , { 1,  5,  7} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  5,  3} , { 3,  5,  7} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9,  7,  8} , { 9,  5,  7} , {10,  1,  2} , {-1, -1, -1} , {-1, -1, -1} },
    {{ 10,  1,  2} , { 9,  5,  0} , { 5,  3,  0} , { 5,  7,  3} , {-1, -1, -1} },
    {{  8,  0,  2} , { 8,  2,  5} , { 8,  5,  7} , {10,  5,  2} , {-1, -1, -1} },
    {{  2, 10,  5} , { 2,  5,  3} , { 3,  5,  7} , {-1, -1, -1} , {-1, -1, -1} },
    {{  7,  9,  5} , { 7,  8,  9} , { 3, 11,  2} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9,  5,  7} , { 9,  7,  2} , { 9,  2,  0} , { 2,  7, 11} , {-1, -1, -1} },
    {{  2,  3, 11} , { 0,  1,  8} , { 1,  7,  8} , { 1,  5,  7} , {-1, -1, -1} },
    {{ 11,  2,  1} , {11,  1,  7} , { 7,  1,  5} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9,  5,  8} , { 8,  5,  7} , {10,  1,  3} , {10,  3, 11} , {-1, -1, -1} },
    {{  5,  7,  0} , { 5,  0,  9} , { 7, 11,  0} , { 1,  0, 10} , {11, 10,  0} },
    {{ 11, 10,  0} , {11,  0,  3} , {10,  5,  0} , { 8,  0,  7} , { 5,  7,  0} },
    {{ 11, 10,  5} , { 7, 11,  5} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{ 10,  6,  5} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  8,  3} , { 5, 10,  6} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9,  0,  1} , { 5, 10,  6} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  8,  3} , { 1,  9,  8} , { 5, 10,  6} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  6,  5} , { 2,  6,  1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  6,  5} , { 1,  2,  6} , { 3,  0,  8} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9,  6,  5} , { 9,  0,  6} , { 0,  2,  6} , {-1, -1, -1} , {-1, -1, -1} },
    {{  5,  9,  8} , { 5,  8,  2} , { 5,  2,  6} , { 3,  2,  8} , {-1, -1, -1} },
    {{  2,  3, 11} , {10,  6,  5} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{ 11,  0,  8} , {11,  2,  0} , {10,  6,  5} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  1,  9} , { 2,  3, 11} , { 5, 10,  6} , {-1, -1, -1} , {-1, -1, -1} },
    {{  5, 10,  6} , { 1,  9,  2} , { 9, 11,  2} , { 9,  8, 11} , {-1, -1, -1} },
    {{  6,  3, 11} , { 6,  5,  3} , { 5,  1,  3} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  8, 11} , { 0, 11,  5} , { 0,  5,  1} , { 5, 11,  6} , {-1, -1, -1} },
    {{  3, 11,  6} , { 0,  3,  6} , { 0,  6,  5} , { 0,  5,  9} , {-1, -1, -1} },
    {{  6,  5,  9} , { 6,  9, 11} , {11,  9,  8} , {-1, -1, -1} , {-1, -1, -1} },
    {{  5, 10,  6} , { 4,  7,  8} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  4,  3,  0} , { 4,  7,  3} , { 6,  5, 10} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  9,  0} , { 5, 10,  6} , { 8,  4,  7} , {-1, -1, -1} , {-1, -1, -1} },
    {{ 10,  6,  5} , { 1,  9,  7} , { 1,  7,  3} , { 7,  9,  4} , {-1, -1, -1} },
    {{  6,  1,  2} , { 6,  5,  1} , { 4,  7,  8} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  2,  5} , { 5,  2,  6} , { 3,  0,  4} , { 3,  4,  7} , {-1, -1, -1} },
    {{  8,  4,  7} , { 9,  0,  5} , { 0,  6,  5} , { 0,  2,  6} , {-1, -1, -1} },
    {{  7,  3,  9} , { 7,  9,  4} , { 3,  2,  9} , { 5,  9,  6} , { 2,  6,  9} },
    {{  3, 11,  2} , { 7,  8,  4} , {10,  6,  5} , {-1, -1, -1} , {-1, -1, -1} },
    {{  5, 10,  6} , { 4,  7,  2} , { 4,  2,  0} , { 2,  7, 11} , {-1, -1, -1} },
    {{  0,  1,  9} , { 4,  7,  8} , { 2,  3, 11} , { 5, 10,  6} , {-1, -1, -1} },
    {{  9,  2,  1} , { 9, 11,  2} , { 9,  4, 11} , { 7, 11,  4} , { 5, 10,  6} },
    {{  8,  4,  7} , { 3, 11,  5} , { 3,  5,  1} , { 5, 11,  6} , {-1, -1, -1} },
    {{  5,  1, 11} , { 5, 11,  6} , { 1,  0, 11} , { 7, 11,  4} , { 0,  4, 11} },
    {{  0,  5,  9} , { 0,  6,  5} , { 0,  3,  6} , {11,  6,  3} , { 8,  4,  7} },
    {{  6,  5,  9} , { 6,  9, 11} , { 4,  7,  9} , { 7, 11,  9} , {-1, -1, -1} },
    {{ 10,  4,  9} , { 6,  4, 10} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  4, 10,  6} , { 4,  9, 10} , { 0,  8,  3} , {-1, -1, -1} , {-1, -1, -1} },
    {{ 10,  0,  1} , {10,  6,  0} , { 6,  4,  0} , {-1, -1, -1} , {-1, -1, -1} },
    {{  8,  3,  1} , { 8,  1,  6} , { 8,  6,  4} , { 6,  1, 10} , {-1, -1, -1} },
    {{  1,  4,  9} , { 1,  2,  4} , { 2,  6,  4} , {-1, -1, -1} , {-1, -1, -1} },
    {{  3,  0,  8} , { 1,  2,  9} , { 2,  4,  9} , { 2,  6,  4} , {-1, -1, -1} },
    {{  0,  2,  4} , { 4,  2,  6} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  8,  3,  2} , { 8,  2,  4} , { 4,  2,  6} , {-1, -1, -1} , {-1, -1, -1} },
    {{ 10,  4,  9} , {10,  6,  4} , {11,  2,  3} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  8,  2} , { 2,  8, 11} , { 4,  9, 10} , { 4, 10,  6} , {-1, -1, -1} },
    {{  3, 11,  2} , { 0,  1,  6} , { 0,  6,  4} , { 6,  1, 10} , {-1, -1, -1} },
    {{  6,  4,  1} , { 6,  1, 10} , { 4,  8,  1} , { 2,  1, 11} , { 8, 11,  1} },
    {{  9,  6,  4} , { 9,  3,  6} , { 9,  1,  3} , {11,  6,  3} , {-1, -1, -1} },
    {{  8, 11,  1} , { 8,  1,  0} , {11,  6,  1} , { 9,  1,  4} , { 6,  4,  1} },
    {{  3, 11,  6} , { 3,  6,  0} , { 0,  6,  4} , {-1, -1, -1} , {-1, -1, -1} },
    {{  6,  4,  8} , {11,  6,  8} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  7, 10,  6} , { 7,  8, 10} , { 8,  9, 10} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  7,  3} , { 0, 10,  7} , { 0,  9, 10} , { 6,  7, 10} , {-1, -1, -1} },
    {{ 10,  6,  7} , { 1, 10,  7} , { 1,  7,  8} , { 1,  8,  0} , {-1, -1, -1} },
    {{ 10,  6,  7} , {10,  7,  1} , { 1,  7,  3} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  2,  6} , { 1,  6,  8} , { 1,  8,  9} , { 8,  6,  7} , {-1, -1, -1} },
    {{  2,  6,  9} , { 2,  9,  1} , { 6,  7,  9} , { 0,  9,  3} , { 7,  3,  9} },
    {{  7,  8,  0} , { 7,  0,  6} , { 6,  0,  2} , {-1, -1, -1} , {-1, -1, -1} },
    {{  7,  3,  2} , { 6,  7,  2} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  2,  3, 11} , {10,  6,  8} , {10,  8,  9} , { 8,  6,  7} , {-1, -1, -1} },
    {{  2,  0,  7} , { 2,  7, 11} , { 0,  9,  7} , { 6,  7, 10} , { 9, 10,  7} },
    {{  1,  8,  0} , { 1,  7,  8} , { 1, 10,  7} , { 6,  7, 10} , { 2,  3, 11} },
    {{ 11,  2,  1} , {11,  1,  7} , {10,  6,  1} , { 6,  7,  1} , {-1, -1, -1} },
    {{  8,  9,  6} , { 8,  6,  7} , { 9,  1,  6} , {11,  6,  3} , { 1,  3,  6} },
    {{  0,  9,  1} , {11,  6,  7} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  7,  8,  0} , { 7,  0,  6} , { 3, 11,  0} , {11,  6,  0} , {-1, -1, -1} },
    {{  7, 11,  6} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  7,  6, 11} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  3,  0,  8} , {11,  7,  6} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  1,  9} , {11,  7,  6} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  8,  1,  9} , { 8,  3,  1} , {11,  7,  6} , {-1, -1, -1} , {-1, -1, -1} },
    {{ 10,  1,  2} , { 6, 11,  7} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  2, 10} , { 3,  0,  8} , { 6, 11,  7} , {-1, -1, -1} , {-1, -1, -1} },
    {{  2,  9,  0} , { 2, 10,  9} , { 6, 11,  7} , {-1, -1, -1} , {-1, -1, -1} },
    {{  6, 11,  7} , { 2, 10,  3} , {10,  8,  3} , {10,  9,  8} , {-1, -1, -1} },
    {{  7,  2,  3} , { 6,  2,  7} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  7,  0,  8} , { 7,  6,  0} , { 6,  2,  0} , {-1, -1, -1} , {-1, -1, -1} },
    {{  2,  7,  6} , { 2,  3,  7} , { 0,  1,  9} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  6,  2} , { 1,  8,  6} , { 1,  9,  8} , { 8,  7,  6} , {-1, -1, -1} },
    {{ 10,  7,  6} , {10,  1,  7} , { 1,  3,  7} , {-1, -1, -1} , {-1, -1, -1} },
    {{ 10,  7,  6} , { 1,  7, 10} , { 1,  8,  7} , { 1,  0,  8} , {-1, -1, -1} },
    {{  0,  3,  7} , { 0,  7, 10} , { 0, 10,  9} , { 6, 10,  7} , {-1, -1, -1} },
    {{  7,  6, 10} , { 7, 10,  8} , { 8, 10,  9} , {-1, -1, -1} , {-1, -1, -1} },
    {{  6,  8,  4} , {11,  8,  6} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  3,  6, 11} , { 3,  0,  6} , { 0,  4,  6} , {-1, -1, -1} , {-1, -1, -1} },
    {{  8,  6, 11} , { 8,  4,  6} , { 9,  0,  1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9,  4,  6} , { 9,  6,  3} , { 9,  3,  1} , {11,  3,  6} , {-1, -1, -1} },
    {{  6,  8,  4} , { 6, 11,  8} , { 2, 10,  1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  2, 10} , { 3,  0, 11} , { 0,  6, 11} , { 0,  4,  6} , {-1, -1, -1} },
    {{  4, 11,  8} , { 4,  6, 11} , { 0,  2,  9} , { 2, 10,  9} , {-1, -1, -1} },
    {{ 10,  9,  3} , {10,  3,  2} , { 9,  4,  3} , {11,  3,  6} , { 4,  6,  3} },
    {{  8,  2,  3} , { 8,  4,  2} , { 4,  6,  2} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  4,  2} , { 4,  6,  2} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  9,  0} , { 2,  3,  4} , { 2,  4,  6} , { 4,  3,  8} , {-1, -1, -1} },
    {{  1,  9,  4} , { 1,  4,  2} , { 2,  4,  6} , {-1, -1, -1} , {-1, -1, -1} },
    {{  8,  1,  3} , { 8,  6,  1} , { 8,  4,  6} , { 6, 10,  1} , {-1, -1, -1} },
    {{ 10,  1,  0} , {10,  0,  6} , { 6,  0,  4} , {-1, -1, -1} , {-1, -1, -1} },
    {{  4,  6,  3} , { 4,  3,  8} , { 6, 10,  3} , { 0,  3,  9} , {10,  9,  3} },
    {{ 10,  9,  4} , { 6, 10,  4} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  4,  9,  5} , { 7,  6, 11} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  8,  3} , { 4,  9,  5} , {11,  7,  6} , {-1, -1, -1} , {-1, -1, -1} },
    {{  5,  0,  1} , { 5,  4,  0} , { 7,  6, 11} , {-1, -1, -1} , {-1, -1, -1} },
    {{ 11,  7,  6} , { 8,  3,  4} , { 3,  5,  4} , { 3,  1,  5} , {-1, -1, -1} },
    {{  9,  5,  4} , {10,  1,  2} , { 7,  6, 11} , {-1, -1, -1} , {-1, -1, -1} },
    {{  6, 11,  7} , { 1,  2, 10} , { 0,  8,  3} , { 4,  9,  5} , {-1, -1, -1} },
    {{  7,  6, 11} , { 5,  4, 10} , { 4,  2, 10} , { 4,  0,  2} , {-1, -1, -1} },
    {{  3,  4,  8} , { 3,  5,  4} , { 3,  2,  5} , {10,  5,  2} , {11,  7,  6} },
    {{  7,  2,  3} , { 7,  6,  2} , { 5,  4,  9} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9,  5,  4} , { 0,  8,  6} , { 0,  6,  2} , { 6,  8,  7} , {-1, -1, -1} },
    {{  3,  6,  2} , { 3,  7,  6} , { 1,  5,  0} , { 5,  4,  0} , {-1, -1, -1} },
    {{  6,  2,  8} , { 6,  8,  7} , { 2,  1,  8} , { 4,  8,  5} , { 1,  5,  8} },
    {{  9,  5,  4} , {10,  1,  6} , { 1,  7,  6} , { 1,  3,  7} , {-1, -1, -1} },
    {{  1,  6, 10} , { 1,  7,  6} , { 1,  0,  7} , { 8,  7,  0} , { 9,  5,  4} },
    {{  4,  0, 10} , { 4, 10,  5} , { 0,  3, 10} , { 6, 10,  7} , { 3,  7, 10} },
    {{  7,  6, 10} , { 7, 10,  8} , { 5,  4, 10} , { 4,  8, 10} , {-1, -1, -1} },
    {{  6,  9,  5} , { 6, 11,  9} , {11,  8,  9} , {-1, -1, -1} , {-1, -1, -1} },
    {{  3,  6, 11} , { 0,  6,  3} , { 0,  5,  6} , { 0,  9,  5} , {-1, -1, -1} },
    {{  0, 11,  8} , { 0,  5, 11} , { 0,  1,  5} , { 5,  6, 11} , {-1, -1, -1} },
    {{  6, 11,  3} , { 6,  3,  5} , { 5,  3,  1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  2, 10} , { 9,  5, 11} , { 9, 11,  8} , {11,  5,  6} , {-1, -1, -1} },
    {{  0, 11,  3} , { 0,  6, 11} , { 0,  9,  6} , { 5,  6,  9} , { 1,  2, 10} },
    {{ 11,  8,  5} , {11,  5,  6} , { 8,  0,  5} , {10,  5,  2} , { 0,  2,  5} },
    {{  6, 11,  3} , { 6,  3,  5} , { 2, 10,  3} , {10,  5,  3} , {-1, -1, -1} },
    {{  5,  8,  9} , { 5,  2,  8} , { 5,  6,  2} , { 3,  8,  2} , {-1, -1, -1} },
    {{  9,  5,  6} , { 9,  6,  0} , { 0,  6,  2} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  5,  8} , { 1,  8,  0} , { 5,  6,  8} , { 3,  8,  2} , { 6,  2,  8} },
    {{  1,  5,  6} , { 2,  1,  6} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  3,  6} , { 1,  6, 10} , { 3,  8,  6} , { 5,  6,  9} , { 8,  9,  6} },
    {{ 10,  1,  0} , {10,  0,  6} , { 9,  5,  0} , { 5,  6,  0} , {-1, -1, -1} },
    {{  0,  3,  8} , { 5,  6, 10} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{ 10,  5,  6} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{ 11,  5, 10} , { 7,  5, 11} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{ 11,  5, 10} , {11,  7,  5} , { 8,  3,  0} , {-1, -1, -1} , {-1, -1, -1} },
    {{  5, 11,  7} , { 5, 10, 11} , { 1,  9,  0} , {-1, -1, -1} , {-1, -1, -1} },
    {{ 10,  7,  5} , {10, 11,  7} , { 9,  8,  1} , { 8,  3,  1} , {-1, -1, -1} },
    {{ 11,  1,  2} , {11,  7,  1} , { 7,  5,  1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  8,  3} , { 1,  2,  7} , { 1,  7,  5} , { 7,  2, 11} , {-1, -1, -1} },
    {{  9,  7,  5} , { 9,  2,  7} , { 9,  0,  2} , { 2, 11,  7} , {-1, -1, -1} },
    {{  7,  5,  2} , { 7,  2, 11} , { 5,  9,  2} , { 3,  2,  8} , { 9,  8,  2} },
    {{  2,  5, 10} , { 2,  3,  5} , { 3,  7,  5} , {-1, -1, -1} , {-1, -1, -1} },
    {{  8,  2,  0} , { 8,  5,  2} , { 8,  7,  5} , {10,  2,  5} , {-1, -1, -1} },
    {{  9,  0,  1} , { 5, 10,  3} , { 5,  3,  7} , { 3, 10,  2} , {-1, -1, -1} },
    {{  9,  8,  2} , { 9,  2,  1} , { 8,  7,  2} , {10,  2,  5} , { 7,  5,  2} },
    {{  1,  3,  5} , { 3,  7,  5} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  8,  7} , { 0,  7,  1} , { 1,  7,  5} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9,  0,  3} , { 9,  3,  5} , { 5,  3,  7} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9,  8,  7} , { 5,  9,  7} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  5,  8,  4} , { 5, 10,  8} , {10, 11,  8} , {-1, -1, -1} , {-1, -1, -1} },
    {{  5,  0,  4} , { 5, 11,  0} , { 5, 10, 11} , {11,  3,  0} , {-1, -1, -1} },
    {{  0,  1,  9} , { 8,  4, 10} , { 8, 10, 11} , {10,  4,  5} , {-1, -1, -1} },
    {{ 10, 11,  4} , {10,  4,  5} , {11,  3,  4} , { 9,  4,  1} , { 3,  1,  4} },
    {{  2,  5,  1} , { 2,  8,  5} , { 2, 11,  8} , { 4,  5,  8} , {-1, -1, -1} },
    {{  0,  4, 11} , { 0, 11,  3} , { 4,  5, 11} , { 2, 11,  1} , { 5,  1, 11} },
    {{  0,  2,  5} , { 0,  5,  9} , { 2, 11,  5} , { 4,  5,  8} , {11,  8,  5} },
    {{  9,  4,  5} , { 2, 11,  3} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  2,  5, 10} , { 3,  5,  2} , { 3,  4,  5} , { 3,  8,  4} , {-1, -1, -1} },
    {{  5, 10,  2} , { 5,  2,  4} , { 4,  2,  0} , {-1, -1, -1} , {-1, -1, -1} },
    {{  3, 10,  2} , { 3,  5, 10} , { 3,  8,  5} , { 4,  5,  8} , { 0,  1,  9} },
    {{  5, 10,  2} , { 5,  2,  4} , { 1,  9,  2} , { 9,  4,  2} , {-1, -1, -1} },
    {{  8,  4,  5} , { 8,  5,  3} , { 3,  5,  1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  4,  5} , { 1,  0,  5} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  8,  4,  5} , { 8,  5,  3} , { 9,  0,  5} , { 0,  3,  5} , {-1, -1, -1} },
    {{  9,  4,  5} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  4, 11,  7} , { 4,  9, 11} , { 9, 10, 11} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  8,  3} , { 4,  9,  7} , { 9, 11,  7} , { 9, 10, 11} , {-1, -1, -1} },
    {{  1, 10, 11} , { 1, 11,  4} , { 1,  4,  0} , { 7,  4, 11} , {-1, -1, -1} },
    {{  3,  1,  4} , { 3,  4,  8} , { 1, 10,  4} , { 7,  4, 11} , {10, 11,  4} },
    {{  4, 11,  7} , { 9, 11,  4} , { 9,  2, 11} , { 9,  1,  2} , {-1, -1, -1} },
    {{  9,  7,  4} , { 9, 11,  7} , { 9,  1, 11} , { 2, 11,  1} , { 0,  8,  3} },
    {{ 11,  7,  4} , {11,  4,  2} , { 2,  4,  0} , {-1, -1, -1} , {-1, -1, -1} },
    {{ 11,  7,  4} , {11,  4,  2} , { 8,  3,  4} , { 3,  2,  4} , {-1, -1, -1} },
    {{  2,  9, 10} , { 2,  7,  9} , { 2,  3,  7} , { 7,  4,  9} , {-1, -1, -1} },
    {{  9, 10,  7} , { 9,  7,  4} , {10,  2,  7} , { 8,  7,  0} , { 2,  0,  7} },
    {{  3,  7, 10} , { 3, 10,  2} , { 7,  4, 10} , { 1, 10,  0} , { 4,  0, 10} },
    {{  1, 10,  2} , { 8,  7,  4} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  4,  9,  1} , { 4,  1,  7} , { 7,  1,  3} , {-1, -1, -1} , {-1, -1, -1} },
    {{  4,  9,  1} , { 4,  1,  7} , { 0,  8,  1} , { 8,  7,  1} , {-1, -1, -1} },
    {{  4,  0,  3} , { 7,  4,  3} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  4,  8,  7} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9, 10,  8} , {10, 11,  8} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  3,  0,  9} , { 3,  9, 11} , {11,  9, 10} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  1, 10} , { 0, 10,  8} , { 8, 10, 11} , {-1, -1, -1} , {-1, -1, -1} },
    {{  3,  1, 10} , {11,  3, 10} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  2, 11} , { 1, 11,  9} , { 9, 11,  8} , {-1, -1, -1} , {-1, -1, -1} },
    {{  3,  0,  9} , { 3,  9, 11} , { 1,  2,  9} , { 2, 11,  9} , {-1, -1, -1} },
    {{  0,  2, 11} , { 8,  0, 11} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  3,  2, 11} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  2,  3,  8} , { 2,  8, 10} , {10,  8,  9} , {-1, -1, -1} , {-1, -1, -1} },
    {{  9, 10,  2} , { 0,  9,  2} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  2,  3,  8} , { 2,  8, 10} , { 0,  1,  8} , { 1, 10,  8} , {-1, -1, -1} },
    {{  1, 10,  2} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  1,  3,  8} , { 9,  1,  8} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  9,  1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{  0,  3,  8} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} },
    {{ -1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} , {-1, -1, -1} }
};


/*
 * Now we have a way to lookup whether a cell (unit cube) needs polygons and where
 * based on a density function.
 * What now?
 * We want to tell if in a particular point of space there is ground and how to create it.
 * In order to do that we need:
 *  - A world coordinate (x,y,z) 
 *  - Implementation of density function
 *
 * A little bit of explanation here:
 * Our world is divided in blocks/cells which are cubes
 * Each cube can be accessed via x,y,z coordinates.
 *
 * With that in mind, we can want to find out for each corner of the block (8) whether is
 * inside, outside or is surface.
 * // pseudocode
 * CubeBit = 0x0000
 * for CubeVertexIndex in 0..7:
 *    Density = DensityFunction(x,y,z)
 *    Flag    = density > 0.0 ? (0x0001 << CubeVertexIndex): 0x00
 *    CubeBit = CubeBit | Flag
 * NumberOfPolygons = LookupNumPolygon[CubeBit]
 * if (NumberOfPolygons > 0)
 *    for PolygonIndex in 0..NumberOfPolygons:
 *      e0 = LookupPolygonEdges[CubeBit][PolygonIndex][0]
 *      e1 = LookupPolygonEdges[CubeBit][PolygonIndex][1]
 *      e2 = LookupPolygonEdges[CubeBit][PolygonIndex][2]
 *
 * Is that it...?
 * Not really, the world will be very edgy if we only query once per block.
 * In reality we need to subdivide each block in many sub-blocks (voxels from now on).
 *
 * Technically, the more the better but it comes with a high cost.
 * In GPU rendering you can get away with 32x32x32 voxels (32 768 voxels)
 * This means that first checks if a block has to be subdivided (meaning not all edges
 * are either - or +).
 * Then for each voxel perform same density queries and check if any polygon has to be created.
 * We end up with a list of triangles.
 *
 * This is a learning purpose CPU implementation.
 * We can't achive such a granularity for large areas, so we are limiting voxels
 * to 3x3x3 = 27
 * How big the cell buffer must be:
 * 27 voxels * 5 triangles    = 135 triangles
 * 135 triangles * 3 vertices = 405 vertices
 * vertex is defined as:
 * 
 * struct vertex_point 
 * {
 *    v3 P; // 3 * 4 = 12
 *    v3 N; // 3 * 4 = 12
 *    v4 Color; // 4 * 4 = 16
 * };
 * Total size must take in consideration aligment. In my machine this is 40 bytes.
 * sizeof(vertex_point) * vertices = 40 * 405 = 16200 bytes per block
 * This is unrealistic because no block will require 5 triangles for each voxel
 *
 * Say we want to commit 5MB of memory for ground.
 * How many blocks can we create?
 * 5MB = 5 * 1024 * 1024 = 5 242 880 bytes
 * 5 242 880 / 16200 (bytes / block) = 323 blocks
 */


/*
 * Each edge has 2 points, start and end.
 * This is the lookup to tell for a given edge which 2 indexes
 */
u32 LookupEgdePoints[12][2] =
{
    {0,1}, {1,2}, {2,3}, {3,0}, // front face
    {4,5}, {5,6}, {6,7}, {7,4}, // back face
    {0,4}, {1,5}, {2,6}, {3,7}  // side
};

r32
DensityAtP(v3 P)
{

    //r32 Density = perlin(P.x, P.y, P.z);
#if 1
    r32 Density = -P.y;

    r32 Freq = 1.1f;
    r32 Scale = 1.0f;
    Density += perlin(P.x * Freq, P.y * Freq, P.z * Freq) * Scale;

    Freq = 1.96f;
    Scale = 0.5f;
    Density += perlin(P.x * Freq, P.y * Freq, P.z * Freq) * Scale;

    Freq = 4.03f;
    Scale = 0.25f;
    Density += perlin(P.x * Freq, P.y * Freq, P.z * Freq) * Scale;
#endif

    return Density;
}

r32
DensityAtP(r32 x, r32 y , r32 z)
{
    r32 Density = DensityAtP(V3(x,y,z));
    return Density;
}

u32
GetDensityIndex(r32 x, r32 y , r32 z, r32 Incr, r32 * VertexDensity = 0)
{
    u32 CubeBitSet = 0;

    v3 P = V3(x, y, z);

    v3 p0 = P + V3(-Incr, -Incr, -Incr); 
    v3 p1 = P + V3(-Incr,  Incr, -Incr); 
    v3 p2 = P + V3( Incr,  Incr, -Incr); 
    v3 p3 = P + V3( Incr, -Incr, -Incr); 
    v3 p4 = P + V3(-Incr, -Incr,  Incr); 
    v3 p5 = P + V3(-Incr,  Incr,  Incr); 
    v3 p6 = P + V3( Incr,  Incr,  Incr); 
    v3 p7 = P + V3( Incr, -Incr,  Incr); 

    r32 DensityP0 = DensityAtP(p0.x, p0.y, p0.z);
    r32 DensityP1 = DensityAtP(p1.x, p1.y, p1.z);
    r32 DensityP2 = DensityAtP(p2.x, p2.y, p2.z);
    r32 DensityP3 = DensityAtP(p3.x, p3.y, p3.z);
    r32 DensityP4 = DensityAtP(p4.x, p4.y, p4.z);
    r32 DensityP5 = DensityAtP(p5.x, p5.y, p5.z);
    r32 DensityP6 = DensityAtP(p6.x, p6.y, p6.z);
    r32 DensityP7 = DensityAtP(p7.x, p7.y, p7.z);

    if (VertexDensity)
    {
        VertexDensity[0] = DensityP0;
        VertexDensity[1] = DensityP1;
        VertexDensity[2] = DensityP2;
        VertexDensity[3] = DensityP3;
        VertexDensity[4] = DensityP4;
        VertexDensity[5] = DensityP5;
        VertexDensity[6] = DensityP6;
        VertexDensity[7] = DensityP7;
    }

    if (DensityP0 > 0.0f) CubeBitSet |= 0x01;
    if (DensityP1 > 0.0f) CubeBitSet |= 0x02;
    if (DensityP2 > 0.0f) CubeBitSet |= 0x04;
    if (DensityP3 > 0.0f) CubeBitSet |= 0x08;
    if (DensityP4 > 0.0f) CubeBitSet |= 0x10;
    if (DensityP5 > 0.0f) CubeBitSet |= 0x20;
    if (DensityP6 > 0.0f) CubeBitSet |= 0x40;
    if (DensityP7 > 0.0f) CubeBitSet |= 0x80;

    return CubeBitSet;
}

u32
GetDensityIndex(v3 P, r32 Incr, r32 * VertexDensity = 0)
{
    u32 CubeBitSet = GetDensityIndex(P.x, P.y, P.z, Incr, VertexDensity);

    return CubeBitSet;
}

v3
GetIncrementForVertex(i32 VertexIndex,r32 Incr)
{
    v3 Result;
#if 0
    switch (VertexIndex)
    {
        case 0: {Result = V3(0,0,0);};break;
        case 1: Result = V3(0,Increment,0);break;
        case 2: Result = V3(Increment,Increment,0);break;
        case 3: Result = V3(Increment,0,0);break;

        case 4: {Result = V3(0,0,Increment);};break;
        case 5: Result = V3(0,Increment,Increment);break;
        case 6: Result = V3(Increment,Increment,Increment);break;
        case 7: Result = V3(Increment,0,Increment);break;
        default:
        {
            Assert(0);
        };
    }
#else

    switch (VertexIndex)
    {
        case 0:{ Result = V3(-Incr, -Incr, -Incr);};break; 
        case 1:{ Result = V3(-Incr,  Incr, -Incr);};break; 
        case 2:{ Result = V3( Incr,  Incr, -Incr);};break; 
        case 3:{ Result = V3( Incr, -Incr, -Incr);};break; 
        case 4:{ Result = V3(-Incr, -Incr,  Incr);};break; 
        case 5:{ Result = V3(-Incr,  Incr,  Incr);};break; 
        case 6:{ Result = V3( Incr,  Incr,  Incr);};break; 
        case 7:{ Result = V3( Incr, -Incr,  Incr);};break; 
    };
#endif
    return Result;
}

u32
CopyCubeToBuffer(vertex_point * Vertices, r32 Size)
{
    r32 cube_triangle_vertex[] = 
    {
        -1.0f,-1.0f,-1.0f, // triangle 1 : begin
        -1.0f,-1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, // triangle 1 : end
        1.0f, 1.0f,-1.0f, // triangle 2 : begin
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f, // triangle 2 : end
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f,-1.0f,
        1.0f,-1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f,-1.0f, 1.0f
    };

    u32 CubeListIndex = 0;
    for (u32 VertexIndex = 0;
            VertexIndex < ArrayCount(cube_triangle_vertex);
            ++VertexIndex)
    {
        Vertices[VertexIndex].P.x = cube_triangle_vertex[CubeListIndex++]*Size;
        Vertices[VertexIndex].P.y = cube_triangle_vertex[CubeListIndex++]*Size;
        Vertices[VertexIndex].P.z = cube_triangle_vertex[CubeListIndex++]*Size;
    }

    return (6*2*3);
}


v3
InterpolateEdgePosition(v3 StartP, v3 EndP, r32 Delta, u32 MaxAttempts)
{
    r32 Density = Delta + 0.1f;

    u32 CurrentAttempt = 1;

    v3 TestP = {};

    for (u32 Attempt = 0; 
             Attempt < MaxAttempts; 
             ++Attempt)
    {
        r32 DensityA = DensityAtP(StartP);
        r32 DensityB = DensityAtP(EndP);

        // linear interpolation
        r32 w = (0.0f - DensityA) / (DensityB - DensityA);

        TestP = StartP + w * (EndP - StartP);

        Density = DensityAtP(TestP);

        if (fabs(Density) < Delta)
        {
            break;
        }
        else
        {
            if (DensityA > DensityB)
            {
                if (Density > 0.0f)
                {
                    StartP = TestP;
                }
                else
                {
                    EndP = TestP;
                }
            }
            else
            {
                if (Density > 0.0f)
                {
                    EndP = TestP;
                }
                else
                {
                    StartP = TestP;
                }
            }
        }

    }

    return TestP;
}


u32
FillBufferTestGround(memory_arena * TempArena, world_pos WorldP, u32 VertexBufferBeginOffset)
{
    u32 VoxelsPerAxis = 3;
    u32 MaxTrianglesPerVoxel = 5;

    r32 OneOverVoxelPerAxis =  (1.0f / VoxelsPerAxis);
    r32 VoxelDim = OneOverVoxelPerAxis;
    r32 HalfVoxelDim = VoxelDim * 0.5f;

    u32 TotalVoxelsPerBlock = (VoxelsPerAxis * VoxelsPerAxis * VoxelsPerAxis);
    u32 MaxTrianglesPerBlock = TotalVoxelsPerBlock * MaxTrianglesPerVoxel;
    u32 MaxVerticesPerBlock = MaxTrianglesPerBlock * 3;

    u32 BufferSizePerBlock = sizeof(vertex_point) * MaxVerticesPerBlock;

    // each 3 vertex == triangle
    vertex_point * Vertices = PushArray(TempArena, MaxVerticesPerBlock,vertex_point);

    // Center block as origin
    v3 P = V3((r32)WorldP.x,(r32)WorldP.y,(r32)WorldP.z) + V3(0.5f);

    //u32 Index = GetDensityIndex(P, 0.4999f);
    u32 Index = GetDensityIndex(P, 0.5f);

    //Logn("At (%i,%i,%i) Index %i", WorldP.x,WorldP.y,WorldP.z,Index);

    b32 EarlyExit = false;

    u32 VerticeCount = 0;
    if (InBetweenExcl(Index,0,255))
    {
#if 1
        for (u32 VoxelYIndex = 0;
                VoxelYIndex < VoxelsPerAxis;
                ++VoxelYIndex)
        {
            for (u32 VoxelXIndex = 0;
                    VoxelXIndex < VoxelsPerAxis;
                    ++VoxelXIndex)
            {
                for (u32 VoxelZIndex = 0;
                        VoxelZIndex < VoxelsPerAxis;
                        ++VoxelZIndex)
                {
                    // center of voxel
                    r32 vx = WorldP.x + VoxelXIndex * VoxelDim + HalfVoxelDim;
                    r32 vy = WorldP.y + VoxelYIndex * VoxelDim + HalfVoxelDim;
                    r32 vz = WorldP.z + VoxelZIndex * VoxelDim + HalfVoxelDim;

                    v3 VoxelP = V3(vx,vy,vz);

                    //Logn("Voxel at " STRP, FP(P));

                    r32 VoxelDimReduced = 1.0f;//0.9923f;
                    r32 VertexDensity[8];
                    u32 IndexVoxel = GetDensityIndex(vx,vy,vz,HalfVoxelDim * VoxelDimReduced, &VertexDensity[0]);
                    if (InBetweenExcl(IndexVoxel,0,255))
                    {
                        u32 TotalTriangles = LookupNumPolygon[IndexVoxel];
                        //Logn("Drawing voxel %i %i %i with total triangles %i",VoxelXIndex,VoxelYIndex,VoxelZIndex,TotalTriangles);

                        for (u32 TriangleIndex = 0;
                                TriangleIndex < TotalTriangles;
                                ++TriangleIndex)
                        {
                            i32 * Triangle = LookupPolygonEdges[IndexVoxel][TriangleIndex];
                            i32 Edge0 = Triangle[0];
                            i32 Edge1 = Triangle[1];
                            i32 Edge2 = Triangle[2];

                            for (u32 EdgeIndex = 0;
                                     EdgeIndex < 3;
                                     ++EdgeIndex)
                            {
                                i32 Edge = Triangle[EdgeIndex];
                                u32 p00 = LookupEgdePoints[Edge][0];
                                u32 p01 = LookupEgdePoints[Edge][1];

                                v3 StartP0 = (VoxelP + GetIncrementForVertex(p00,HalfVoxelDim * VoxelDimReduced));
                                v3 FromStartToEnd = (GetIncrementForVertex(p01,HalfVoxelDim * VoxelDimReduced) - GetIncrementForVertex(p00,HalfVoxelDim * VoxelDimReduced));
                                v3 EndP0 = StartP0 + FromStartToEnd;
                                v3 TestP0 = InterpolateEdgePosition(StartP0, EndP0, 1.0f,5);
                                Vertices[VerticeCount].P =  TestP0 - P;
                                ++VerticeCount;
                            }
                            v3 A = Vertices[VerticeCount - 3].P;
                            v3 B = Vertices[VerticeCount - 2].P;
                            v3 C = Vertices[VerticeCount - 1].P;

                            v3 N = -Cross((B - A),(C - A));

                            Vertices[VerticeCount - 3].N =  N;
                            Vertices[VerticeCount - 2].N =  N;
                            Vertices[VerticeCount - 1].N =  N;

#if 0

                            v3 p0 =  StartP0 + w0 *  FromStartToEnd;
                            r32 d0 = DensityAtP(p0);



                            u32 p10 = LookupEgdePoints[Edge1][0];
                            u32 p11 = LookupEgdePoints[Edge1][1];
                            r32 StartEdge1 = VertexDensity[p10];
                            r32 EndEdge1 = VertexDensity[p11];
                            r32 w1 = (0.0f - StartEdge1) / (EndEdge1 - StartEdge1);
                            v3 p1 = (VoxelP + GetIncrementForVertex(p10,HalfVoxelDim * VoxelDimReduced))  + 
                                w1 * (GetIncrementForVertex(p11,HalfVoxelDim * VoxelDimReduced) - GetIncrementForVertex(p10,HalfVoxelDim * VoxelDimReduced));
                            r32 d1 = DensityAtP(p1);


                            u32 p20 = LookupEgdePoints[Edge2][0];
                            u32 p21 = LookupEgdePoints[Edge2][1];
                            r32 StartEdge2 = VertexDensity[p20];
                            r32 EndEdge2 = VertexDensity[p21];
                            r32 w2 = (0.0f - StartEdge2) / (EndEdge2 - StartEdge2);

                            v3 p2 = (VoxelP + GetIncrementForVertex(p20,HalfVoxelDim * VoxelDimReduced))  + 
                                w2 * (GetIncrementForVertex(p21,HalfVoxelDim * VoxelDimReduced) - GetIncrementForVertex(p20,HalfVoxelDim * VoxelDimReduced));
                            r32 d2 = DensityAtP(p2);

                            Vertices[VerticeCount].P =  p0;
                            ++VerticeCount;

                            Vertices[VerticeCount].P = p1;
                            ++VerticeCount;

                            Vertices[VerticeCount].P = p2;
                            ++VerticeCount;
                            Logn("Density...0? %f %f %f",d0,d1,d2);
                            //Logn("Weights 0,1,2: %f %f %f",w0,w1,w2);
#endif


                        }

                        //VerticeCount += CopyCubeToBuffer(Vertices + VerticeCount,VoxelDim);
                        
                        //EarlyExit = true;
                        EarlyExit = false;
                    }
                    if (EarlyExit) break;
                }
                if (EarlyExit) break;
            }
            if (EarlyExit) break;
        }
#else
        for (u32 VoxelYIndex = 0;
                VoxelYIndex < VoxelsPerAxis;
                ++VoxelYIndex)
        {
            for (u32 VoxelXIndex = 0;
                    VoxelXIndex < VoxelsPerAxis;
                    ++VoxelXIndex)
            {
                for (u32 VoxelZIndex = 0;
                        VoxelZIndex < VoxelsPerAxis;
                        ++VoxelZIndex)
                {
                    r32 vx = WorldP.x + VoxelZIndex * OneOverVoxelPerAxis;
                    r32 vy = WorldP.y + VoxelZIndex * OneOverVoxelPerAxis;
                    r32 vz = WorldP.z + VoxelZIndex * OneOverVoxelPerAxis;

                    v3 voxelP = V3(vx,vy,vz);

                    u32 IndexVoxel = GetDensityIndex(vx,vy,vz);
                    if (InBetweenExcl(IndexVoxel,0,255))
                    {
                        r32 VertexDensity[8];
                        for (u32 VertexIndex = 0;
                                VertexIndex < 8;
                                ++VertexIndex)
                        {
                            v3 VertexP = voxelP + GetIncrementForVertex(VertexIndex,OneOverVoxelPerAxis);
                            VertexDensity[VertexIndex] = DensityAtP(VertexP);
                        }
                        u32 TotalTriangles = LookupNumPolygon[IndexVoxel];


                        for (u32 TriangleIndex = 0;
                                TriangleIndex < TotalTriangles;
                                ++TriangleIndex)
                        {
                            i32 * Triangle = LookupPolygonEdges[IndexVoxel][TriangleIndex];
                            i32 Edge0 = Triangle[0];
                            i32 Edge1 = Triangle[1];
                            i32 Edge2 = Triangle[2];

                            u32 p0 = LookupEgdePoints[Edge0][0];
                            r32 StartEdge0 = VertexDensity[p0];
                            r32 EndEdge0 = VertexDensity[LookupEgdePoints[Edge0][1]];
                            r32 w0 = (r32)fabs((0.0f - StartEdge0) / (EndEdge0 - StartEdge0));

                            Vertices[VerticeCount].P = voxelP + w0 * GetIncrementForVertex(p0,OneOverVoxelPerAxis) - P;
                            ++VerticeCount;

                            u32 p1 = LookupEgdePoints[Edge1][0];
                            r32 StartEdge1 = VertexDensity[p1];
                            r32 EndEdge1 = VertexDensity[LookupEgdePoints[Edge1][1]];
                            r32 w1 = (r32)fabs((0.0f - StartEdge1) / (EndEdge1 - StartEdge1));

                            Vertices[VerticeCount].P = voxelP + w1 * GetIncrementForVertex(p1,OneOverVoxelPerAxis) - P;
                            ++VerticeCount;

                            u32 p2 = LookupEgdePoints[Edge2][0];
                            r32 StartEdge2 = VertexDensity[p2];
                            r32 EndEdge2 = VertexDensity[LookupEgdePoints[Edge2][1]];
                            r32 w2 = (r32)fabs((0.0f - StartEdge2) / (EndEdge2 - StartEdge2));

                            Vertices[VerticeCount].P = voxelP + w2 * GetIncrementForVertex(p2,OneOverVoxelPerAxis) - P;
                            ++VerticeCount;
                        }
                    }
                }
            }
        }
#endif
#if 0
        Logn("Total vertices: %i", VerticeCount);
        for (u32 VertexIndex = 0;
                VertexIndex < VerticeCount;
                ++VertexIndex)
        {
            Logn(STRP,FP(Vertices[VertexIndex].P));
        }
#endif
    }

    u32 MeshSize = VerticeCount * sizeof(vertex_point);

    if (MeshSize > 0)
    {
        RenderPushVertexData(Vertices,MeshSize, VertexBufferBeginOffset); 
        //Logn("Mesh size %i",MeshSize);
    }

    return MeshSize;
}

void
TestGround_rangevoxels(game_state * GameState)
{
    i32 Tiles = 1;
    i32 YTiles = 1;

    memory_arena * TempArena = &GameState->TemporaryArena;
    world * World = &GameState->World;

    for (i32 x = -Tiles;
            x < Tiles;
            ++x)
    {
        for (i32 y = -YTiles;
                y < YTiles;
                ++y)
        {
            for (i32 z = -Tiles;
                    z < Tiles;
                    ++z)
            {
                BeginTempArena(TempArena,1);

                world_pos P = WorldPosition(x,y,z);
                u32 VertexBufferBeginOffset = GameState->VertexArena.CurrentSize;
                u32 SizeVertexBuffer = 
                    FillBufferTestGround(&GameState->TemporaryArena, 
                            P,
                            VertexBufferBeginOffset);
                if (SizeVertexBuffer > 0)
                {
                    PushMeshSize(&GameState->VertexArena,SizeVertexBuffer,1);

                    u32 MeshID = GameState->GroundMeshCount++;
                    mesh_group * MeshGroup = GameState->GroundMeshGroup + MeshID;

                    entity * GroundEntity = AddEntity(&GameState->World, P);
                    v3 GroundScale = World->GridCellDimInMeters;
                    EntityAddTranslation(GroundEntity,0,V3(0), GroundScale,0.0f);

                    v3 Color;
                    if ((P.z % 2) == 0)
                    {
                        if ((P.x % 2) == 0)
                        {
                            Color = V3(1.0f,0,0);
                        }
                        else
                        {
                            Color = V3(0.0f,0,1.0f);
                        }
                    }
                    else
                    {
                        if ((P.x % 2) == 0)
                        {
                            Color = V3(0.0f,0,1.0f);
                        }
                        else
                        {
                            Color = V3(1.0f,0,0);
                        }
                    }

                    Color = V3(0.3f,1.0f,0.0f);


                    GroundEntity->Color = Color;
                    GroundEntity->IsGround = true;
                    GroundEntity->MeshObjCount = 1;
                    GroundEntity->MeshID.ID = MeshID;

                    MeshGroup->Loaded = true;
                    MeshGroup->Meshes->VertexSize = SizeVertexBuffer;
                    MeshGroup->Meshes->OffsetVertices = VertexBufferBeginOffset;
#if 1
                    r32 VoxelDim = 1.0f/3.0f;
                    //world_pos CubeP = MapIntoCell(World,P, (World->GridCellDimInMeters*0.5f));// + V3(VoxelDim*0.5f*World->GridCellDimInMeters);
                    world_pos CubeP = P;
                    entity * CubeEntity = AddEntity(&GameState->World, CubeP);
                    EntityAddTranslation(CubeEntity,0,V3(0), V3(VoxelDim),0.0f);
                    CubeEntity->Color = V3(1.0f,0,0);
                    CubeEntity->IsGround = false;
                    EntityAddMesh(CubeEntity,Mesh(0));
#endif

                }

                EndTempArena(TempArena,1);
            }
        }
    }
}

void
TestGround_Only1Voxel(game_state * GameState)
{
    memory_arena * TempArena = &GameState->TemporaryArena;
    world * World = &GameState->World;

    BeginTempArena(TempArena,1);

    world_pos P = WorldPosition(0,0,0);

    u32 VertexBufferBeginOffset = GameState->VertexArena.CurrentSize;
    u32 SizeVertexBuffer = 
        FillBufferTestGround(&GameState->TemporaryArena, 
                P,
                VertexBufferBeginOffset);
    if (SizeVertexBuffer > 0)
    {
        PushMeshSize(&GameState->VertexArena,SizeVertexBuffer,1);

        u32 MeshID = GameState->GroundMeshCount++;
        mesh_group * MeshGroup = GameState->GroundMeshGroup + MeshID;

        entity * GroundEntity = AddEntity(&GameState->World, P);
        v3 GroundScale = World->GridCellDimInMeters;
        EntityAddTranslation(GroundEntity,0,V3(0), GroundScale,0.0f);

        v3 Color;
        if ((P.z % 2) == 0)
        {
            if ((P.x % 2) == 0)
            {
                Color = V3(1.0f,0,0);
            }
            else
            {
                Color = V3(0.0f,0,1.0f);
            }
        }
        else
        {
            if ((P.x % 2) == 0)
            {
                Color = V3(0.0f,0,1.0f);
            }
            else
            {
                Color = V3(1.0f,0,0);
            }
        }

        Color = V3(0.3f,1.0f,0.0f);


        GroundEntity->Color = Color;
        GroundEntity->IsGround = true;
        GroundEntity->MeshObjCount = 1;
        GroundEntity->MeshID.ID = MeshID;

        MeshGroup->Loaded = true;
        MeshGroup->Meshes->VertexSize = SizeVertexBuffer;
        MeshGroup->Meshes->OffsetVertices = VertexBufferBeginOffset;
#if 1
        r32 VoxelDim = 1.0f/3.0f;
        //world_pos CubeP = MapIntoCell(World,P, (World->GridCellDimInMeters*0.5f));// + V3(VoxelDim*0.5f*World->GridCellDimInMeters);
        world_pos CubeP = P;
        entity * CubeEntity = AddEntity(&GameState->World, CubeP);
        EntityAddTranslation(CubeEntity,0,V3(0), V3(VoxelDim),0.0f);
        CubeEntity->Color = V3(1.0f,0,0);
        CubeEntity->IsGround = false;
        EntityAddMesh(CubeEntity,Mesh(0));
#endif

    }

    EndTempArena(TempArena,1);
}

void
TestGround02(game_state * GameState)
{
    memory_arena * TempArena = &GameState->TemporaryArena;
    world * World = &GameState->World;

    BeginTempArena(TempArena,1);

    world_pos P = WorldPosition(0,0,0);

    u32 VertexBufferBeginOffset = GameState->VertexArena.CurrentSize;
    u32 SizeVertexBuffer = 
        FillBufferTestGround(&GameState->TemporaryArena, 
                P,
                VertexBufferBeginOffset);
    if (SizeVertexBuffer > 0)
    {
        PushMeshSize(&GameState->VertexArena,SizeVertexBuffer,1);

        u32 MeshID = GameState->GroundMeshCount++;
        mesh_group * MeshGroup = GameState->GroundMeshGroup + MeshID;

        entity * GroundEntity = AddEntity(&GameState->World, P);
        v3 GroundScale = World->GridCellDimInMeters;
        EntityAddTranslation(GroundEntity,0,V3(0), GroundScale,0.0f);

        v3 Color;
        if ((P.z % 2) == 0)
        {
            if ((P.x % 2) == 0)
            {
                Color = V3(1.0f,0,0);
            }
            else
            {
                Color = V3(0.0f,0,1.0f);
            }
        }
        else
        {
            if ((P.x % 2) == 0)
            {
                Color = V3(0.0f,0,1.0f);
            }
            else
            {
                Color = V3(1.0f,0,0);
            }
        }

        Color = V3(0.3f,1.0f,0.0f);


        GroundEntity->Color = Color;
        GroundEntity->IsGround = true;
        GroundEntity->MeshObjCount = 1;
        GroundEntity->MeshID.ID = MeshID;

        MeshGroup->Loaded = true;
        MeshGroup->Meshes->VertexSize = SizeVertexBuffer;
        MeshGroup->Meshes->OffsetVertices = VertexBufferBeginOffset;
#if 1
        r32 VoxelDim = 1.0f/3.0f;
        //world_pos CubeP = MapIntoCell(World,P, (World->GridCellDimInMeters*0.5f));// + V3(VoxelDim*0.5f*World->GridCellDimInMeters);
        world_pos CubeP = P;
        entity * CubeEntity = AddEntity(&GameState->World, CubeP);
        EntityAddTranslation(CubeEntity,0,V3(0), V3(VoxelDim),0.0f);
        CubeEntity->Color = V3(1.0f,0,0);
        CubeEntity->IsGround = false;
        EntityAddMesh(CubeEntity,Mesh(0));
#endif

    }

    EndTempArena(TempArena,1);
}

u32
FillBufferTestGround2(memory_arena * TempArena, world_pos WorldP, v3 Origen, u32 VoxelsPerAxis = 3)
{
    u32 MaxTrianglesPerVoxel = 5;

    r32 OneOverVoxelPerAxis =  (1.0f / VoxelsPerAxis);
    r32 VoxelDim = OneOverVoxelPerAxis;
    r32 HalfVoxelDim = VoxelDim * 0.5f;

    u32 TotalVoxelsPerBlock = (VoxelsPerAxis * VoxelsPerAxis * VoxelsPerAxis);
    u32 MaxTrianglesPerBlock = TotalVoxelsPerBlock * MaxTrianglesPerVoxel;
    u32 MaxVerticesPerBlock = MaxTrianglesPerBlock * 3;

    u32 BufferSizePerBlock = sizeof(vertex_point) * MaxVerticesPerBlock;

    // each 3 vertex == triangle
    vertex_point * Vertices = PushArray(TempArena, MaxVerticesPerBlock,vertex_point);

    // Center block as origin
    v3 P = V3((r32)WorldP.x,(r32)WorldP.y,(r32)WorldP.z) + V3(0.5f);

    // Do not center block
    //v3 P = V3((r32)WorldP.x,(r32)WorldP.y,(r32)WorldP.z);

    u32 Index = GetDensityIndex(P, 0.5f);

    r32 MaxDist = 0.0f;
    u32 VerticeMaxDist = 0;

    u32 VerticeCount = 0;
    if (InBetweenExcl(Index,0,255))
    {
        //Logn("At (%i,%i,%i) Index %i", WorldP.x,WorldP.y,WorldP.z,Index);
        for (u32 VoxelYIndex = 0;
                VoxelYIndex < VoxelsPerAxis;
                ++VoxelYIndex)
        {
            for (u32 VoxelXIndex = 0;
                    VoxelXIndex < VoxelsPerAxis;
                    ++VoxelXIndex)
            {
                for (u32 VoxelZIndex = 0;
                        VoxelZIndex < VoxelsPerAxis;
                        ++VoxelZIndex)
                {
                    // center of voxel
                    r32 vx = WorldP.x + VoxelXIndex * VoxelDim + HalfVoxelDim;
                    r32 vy = WorldP.y + VoxelYIndex * VoxelDim + HalfVoxelDim;
                    r32 vz = WorldP.z + VoxelZIndex * VoxelDim + HalfVoxelDim;

                    v3 VoxelP = V3(vx,vy,vz);


                    r32 VoxelDimReduced = 1.0f;//0.9923f;
                    r32 VertexDensity[8];
                    u32 IndexVoxel = GetDensityIndex(vx,vy,vz,HalfVoxelDim * VoxelDimReduced, &VertexDensity[0]);
                    if (InBetweenExcl(IndexVoxel,0,255))
                    {
                        u32 TotalTriangles = LookupNumPolygon[IndexVoxel];
                        //Logn("Drawing voxel %i %i %i with total triangles %i",VoxelXIndex,VoxelYIndex,VoxelZIndex,TotalTriangles);

                        for (u32 TriangleIndex = 0;
                                TriangleIndex < TotalTriangles;
                                ++TriangleIndex)
                        {
                            i32 * Triangle = LookupPolygonEdges[IndexVoxel][TriangleIndex];
                            i32 Edge0 = Triangle[0];
                            i32 Edge1 = Triangle[1];
                            i32 Edge2 = Triangle[2];

                            for (u32 EdgeIndex = 0;
                                    EdgeIndex < 3;
                                    ++EdgeIndex)
                            {
                                i32 Edge = Triangle[EdgeIndex];
                                u32 p00 = LookupEgdePoints[Edge][0];
                                u32 p01 = LookupEgdePoints[Edge][1];

                                v3 StartP0 = (VoxelP + GetIncrementForVertex(p00,HalfVoxelDim * VoxelDimReduced));
                                v3 FromStartToEnd = (GetIncrementForVertex(p01,HalfVoxelDim * VoxelDimReduced) - GetIncrementForVertex(p00,HalfVoxelDim * VoxelDimReduced));
                                v3 EndP0 = StartP0 + FromStartToEnd;
                                v3 TestP0 = InterpolateEdgePosition(StartP0, EndP0, 1.0f,5);
                                Vertices[VerticeCount].P =  TestP0 - Origen;
                                ++VerticeCount;

                                r32 DistToOrigen = LengthSqr(TestP0 - P);
                                if (DistToOrigen > MaxDist)
                                {
                                    MaxDist = DistToOrigen;
                                    VerticeMaxDist = VerticeCount - 1;
                                }
                            }
                            v3 A = Vertices[VerticeCount - 3].P;
                            v3 B = Vertices[VerticeCount - 2].P;
                            v3 C = Vertices[VerticeCount - 1].P;

                            v3 N = Cross((B - A),(C - A));

                            Vertices[VerticeCount - 3].N =  N;
                            Vertices[VerticeCount - 2].N =  N;
                            Vertices[VerticeCount - 1].N =  N;
                        }

                        //VerticeCount += CopyCubeToBuffer(Vertices + VerticeCount,VoxelDim);

                    }
                }
            }
        }

        //Logn("Max distance " STRP,FP((Vertices + VerticeMaxDist)->P));
    }

    TempArena->CurrentSize -= sizeof(vertex_point)*(MaxVerticesPerBlock - VerticeCount);

    return VerticeCount;
}

void
TestGroundSingleMeshMultipleVoxel(game_state * GameState)
{
    memory_arena * TempArena = &GameState->TemporaryArena;
    world * World = &GameState->World;

    i32 StartRow = 0;
    i32 MaxCases = StartRow + 11;
    
    world_pos BeginP = WorldPosition(0,0,0);
    v3 Origen = Substract(World, BeginP, WorldPosition((StartRow+MaxCases)/2,0,0));

    for (i32 rows = StartRow; rows < MaxCases; ++ rows)
    {
        BeginTempArena(TempArena,1);

        v3 GroundScale = V3(5.0f);//World->GridCellDimInMeters;
        world_pos DrawP = MapIntoCell(World, BeginP,V3(0,0,(r32)(rows - StartRow)  * GroundScale.z * 2.0f));

        u32 VertexBufferBeginOffset = GameState->VertexArena.CurrentSize;

        vertex_point * Vertices = (vertex_point *)((u8 *)TempArena->Base + TempArena->CurrentSize);

        u32 TotalVertices = 0;
        for (i32 x = rows; x < MaxCases; ++x)
        {
            world_pos WorldP = WorldPosition(x,0,0);
            u32 VerticesCount = FillBufferTestGround2(&GameState->TemporaryArena, WorldP, Origen);
            TotalVertices += VerticesCount;

#if 0
            if (VerticesCount > 0)
            {
                //world_pos CubeP = MapIntoCell(World,DrawP,V3(5.0f*(r32)x,0,0));
                world_pos CubeP = WorldP;
                entity * CubeEntity = AddEntity(&GameState->World, CubeP);
                EntityAddTranslation(CubeEntity,0,V3(0), V3(0.5f),0.0f);
                CubeEntity->Color = V3(1.0f,0,0);
                CubeEntity->IsGround = false;
                EntityAddMesh(CubeEntity,Mesh(0));
            }
#endif
        }

        if (TotalVertices > 0)
        {

            u32 SizeVertexBuffer = sizeof(vertex_point) * TotalVertices;

            PushMeshSize(&GameState->VertexArena,SizeVertexBuffer,1);

            u32 MeshID = GameState->GroundMeshCount++;
            mesh_group * MeshGroup = GameState->GroundMeshGroup + MeshID;

            world_pos GroundEntityP = DrawP;
            entity * GroundEntity = AddEntity(&GameState->World, DrawP);
            EntityAddTranslation(GroundEntity,0,V3(0), GroundScale,0.0f);

            v3 Color;
            Color = V3(0.3f,1.0f,0.0f);

            GroundEntity->Color = Color;
            GroundEntity->IsGround = true;
            GroundEntity->MeshObjCount = 1;
            GroundEntity->MeshID.ID = MeshID;

            MeshGroup->Loaded = true;
            MeshGroup->Meshes->VertexSize = SizeVertexBuffer;
            MeshGroup->Meshes->OffsetVertices = VertexBufferBeginOffset;

            RenderPushVertexData(Vertices,SizeVertexBuffer, VertexBufferBeginOffset); 
        }

        EndTempArena(TempArena,1);
    }
}

void
TestGroundSingleMeshMultipleVoxel2(game_state * GameState)
{
    memory_arena * TempArena = &GameState->TemporaryArena;
    world * World = &GameState->World;

    i32 StartRow = 0;
    i32 MaxCases = StartRow + 11; // should be odd to better align mesh with center
    
    world_pos BeginP = WorldPosition(0,0,0);
    v3 Origen = -Substract(World, BeginP, WorldPosition((StartRow+MaxCases)/2,0,0));

    BeginTempArena(TempArena,1);


    u32 VertexBufferBeginOffset = GameState->VertexArena.CurrentSize;

    vertex_point * Vertices = (vertex_point *)((u8 *)TempArena->Base + TempArena->CurrentSize);

    u32 TotalVertices = 0;

    for (i32 x = StartRow; x < MaxCases; ++x)
    {
        for (i32 y = -1; y < 2; ++y)
        {
            for (i32 z = StartRow; z < MaxCases; ++z)
            {
                world_pos WorldP = WorldPosition(x,y,z);
                u32 VerticesCount = FillBufferTestGround2(&GameState->TemporaryArena, WorldP, Origen,8);
                TotalVertices += VerticesCount;

#if 0
                if (VerticesCount > 0)
                {
                    //world_pos CubeP = MapIntoCell(World,DrawP,V3(5.0f*(r32)x,0,0));
                    world_pos CubeP = WorldP;
                    entity * CubeEntity = AddEntity(&GameState->World, CubeP);
                    EntityAddTranslation(CubeEntity,0,V3(0), V3(0.5f),0.0f);
                    CubeEntity->Color = V3(1.0f,0,0);
                    CubeEntity->IsGround = false;
                    EntityAddMesh(CubeEntity,Mesh(0));
                }
#endif
            }
        }
    }

    if (TotalVertices > 0)
    {

        u32 SizeVertexBuffer = sizeof(vertex_point) * TotalVertices;

        PushMeshSize(&GameState->VertexArena,SizeVertexBuffer,1);

        u32 MeshID = GameState->GroundMeshCount++;
        mesh_group * MeshGroup = GameState->GroundMeshGroup + MeshID;

        entity * GroundEntity = AddEntity(&GameState->World, BeginP);
        v3 GroundScale = V3(5.0f);//World->GridCellDimInMeters;
        EntityAddTranslation(GroundEntity,0,V3(0), GroundScale,0.0f);

        v3 Color;
        Color = V3(0.3f,1.0f,0.0f);

        GroundEntity->Color = Color;
        GroundEntity->IsGround = true;
        GroundEntity->MeshObjCount = 1;
        GroundEntity->MeshID.ID = MeshID;

        MeshGroup->Loaded = true;
        MeshGroup->Meshes->VertexSize = SizeVertexBuffer;
        MeshGroup->Meshes->OffsetVertices = VertexBufferBeginOffset;

        RenderPushVertexData(Vertices,SizeVertexBuffer, VertexBufferBeginOffset); 
    }

    EndTempArena(TempArena,1);
}
