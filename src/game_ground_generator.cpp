#include "game.h"
#include "noise_perlin.cpp"
#include "game_math.h"
#include <limits.h>
#include "game_ground_generator.h"
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
    /*
     * <0 sky, empty space
     *  0 surface point
     * >0 inside of terrain
     */
    // reverse so ground is >0 (inside terrain)
    // and above ground is <0
#if 1
  r32 Density = -P.y;
#else
  // sphere
  r32 rad = 10; 
  r32 Density = rad - Length(P - V3(0, -rad, 0));
#endif

#if 0
    // at most + 1
    r32 Freq = 0.6f;
    r32 Amplitude = 1.0f;
    //Density += perlin(P.x, P.y, P.z);
    //Density += perlin(Freq * P.x,Freq *  P.y,Freq *  P.z);
    r32 calc = perlin(Freq * P.x,Freq *  P.y,Freq *  P.z) * Amplitude;
    Assert(calc >= 0.0f && calc <= 1.0);
    Density += calc;
#endif

#if 1
    r32 Freq = 0.30f;
    r32 Scale = 4.0f;
    Density += perlin(P.x * Freq, P.y * Freq, P.z * Freq) * Scale;

#if 1
    Freq = 0.63f;
    Scale = 2.05f;
    Density += perlin(P.x * Freq, P.y * Freq, P.z * Freq) * Scale;

    Freq = 1.31f;
    Scale = 1.02f;
    Density += perlin(P.x * Freq, P.y * Freq, P.z * Freq) * Scale;

#endif

    //r32 SkyCeiling = 5.0f;
    //Density += (SkyCeiling - P.y);
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
GetDensityIndex(r32 x, r32 y , r32 z, r32 Incr)
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
GetDensityIndexCorner(r32 x, r32 y , r32 z, r32 Incr)
{
    u32 CubeBitSet = 0;

    v3 P = V3(x, y, z);

    v3 p0 = P + V3(0,       0, 0); 
    v3 p1 = P + V3(0,    Incr, 0); 
    v3 p2 = P + V3(Incr, Incr, 0); 
    v3 p3 = P + V3(Incr,    0, 0); 

    v3 p4 = P + V3(0,       0, Incr); 
    v3 p5 = P + V3(0,    Incr, Incr); 
    v3 p6 = P + V3(Incr, Incr, Incr); 
    v3 p7 = P + V3(Incr,    0, Incr); 

    r32 DensityP0 = DensityAtP(p0.x, p0.y, p0.z);
    r32 DensityP1 = DensityAtP(p1.x, p1.y, p1.z);
    r32 DensityP2 = DensityAtP(p2.x, p2.y, p2.z);
    r32 DensityP3 = DensityAtP(p3.x, p3.y, p3.z);
    r32 DensityP4 = DensityAtP(p4.x, p4.y, p4.z);
    r32 DensityP5 = DensityAtP(p5.x, p5.y, p5.z);
    r32 DensityP6 = DensityAtP(p6.x, p6.y, p6.z);
    r32 DensityP7 = DensityAtP(p7.x, p7.y, p7.z);

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
GetDensityIndex(v3 P, r32 Incr)
{
    u32 CubeBitSet = GetDensityIndex(P.x, P.y, P.z, Incr);

    return CubeBitSet;
}

u32
GetDensityIndexCorner(v3 P, r32 Incr)
{
    u32 CubeBitSet = GetDensityIndexCorner(P.x, P.y, P.z, Incr);

    return CubeBitSet;
}

v3
GetIncrementForVertexCorner(i32 VertexIndex,r32 Incr)
{
    v3 Result = {};

    switch (VertexIndex)
    {
        case 0:{ Result = V3(    0,     0,  0);};break; 
        case 1:{ Result = V3(    0,  Incr,  0);};break; 
        case 2:{ Result = V3( Incr,  Incr,  0);};break; 
        case 3:{ Result = V3( Incr,     0,  0);};break; 

        case 4:{ Result = V3(    0,     0,  Incr);};break; 
        case 5:{ Result = V3(    0,  Incr,  Incr);};break; 
        case 6:{ Result = V3( Incr,  Incr,  Incr);};break; 
        case 7:{ Result = V3( Incr,     0,  Incr);};break; 
        default:
               Assert(0);
    };

    return Result;
}
v3
GetIncrementForVertex(i32 VertexIndex,r32 Incr)
{
    v3 Result = {};

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
        default:
               Assert(0);
    };

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


        Assert((DensityB - DensityA) != 0.0f);

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

    //Logn("Final density %f",Density);
    return TestP;
}


u32
FillBufferTestGroundCorner(memory_arena * TempArena, world_pos WorldP, world_pos Origen, u32 VoxelsPerAxis = 3)
{
    u32 MaxTrianglesPerVoxel = 5;

    r32 OneOverVoxelPerAxis =  (1.0f / VoxelsPerAxis);
    r32 VoxelDim = OneOverVoxelPerAxis;

    u32 TotalVoxelsPerBlock = (VoxelsPerAxis * VoxelsPerAxis * VoxelsPerAxis);
    u32 MaxTrianglesPerBlock = TotalVoxelsPerBlock * MaxTrianglesPerVoxel;
    u32 MaxVerticesPerBlock = MaxTrianglesPerBlock * 3;

    u32 BufferSizePerBlock = sizeof(vertex_point) * MaxVerticesPerBlock;

    // each 3 vertex == triangle
    vertex_point * Vertices = PushArray(TempArena, MaxVerticesPerBlock,vertex_point);

    r32 WorldX = (r32)WorldP.x;
    r32 WorldY = (r32)WorldP.y;
    r32 WorldZ = (r32)WorldP.z;

    r32 OffsetX = -(r32)Origen.x;
    r32 OffsetY = -(r32)Origen.y;
    r32 OffsetZ = -(r32)Origen.z;

    v3 OffsetFromOrigin = V3(OffsetX,OffsetY,OffsetZ);

    v3 P = V3(WorldX,WorldY,WorldZ);
    u32 Index = GetDensityIndexCorner(P, 1.0f);

    r32 MaxDist = 0.0f;
    u32 VerticeMaxDist = 0;

    v3 LookupVertexOffset[] = {
        V3(        0,         0,     0), 
        V3(        0,  VoxelDim,     0), 
        V3( VoxelDim,  VoxelDim,     0), 
        V3( VoxelDim,         0,     0), 

        V3(        0,         0,  VoxelDim), 
        V3(        0,  VoxelDim,  VoxelDim), 
        V3( VoxelDim,  VoxelDim,  VoxelDim), 
        V3( VoxelDim,         0,  VoxelDim) 
    };

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
#if 1
                    r32 vx = P.x + VoxelXIndex * VoxelDim;
                    r32 vy = P.y + VoxelYIndex * VoxelDim;
                    r32 vz = P.z + VoxelZIndex * VoxelDim;
#else
                    r32 vx = WorldX + VoxelXIndex * VoxelDim;
                    r32 vy = WorldY + VoxelYIndex * VoxelDim;
                    r32 vz = WorldZ + VoxelZIndex * VoxelDim;
#endif

                    v3 VoxelP = V3(vx,vy,vz);

                    u32 IndexVoxel = GetDensityIndexCorner(vx,vy,vz,VoxelDim);

                    if (InBetweenExcl(IndexVoxel,0,255))
                    {
                        u32 TotalTriangles = LookupNumPolygon[IndexVoxel];
                        //Logn("Drawing voxel %i %i %i with total triangles %i",VoxelXIndex,VoxelYIndex,VoxelZIndex,TotalTriangles);

                        for (u32 TriangleIndex = 0;
                                TriangleIndex < TotalTriangles;
                                ++TriangleIndex)
                        {
                            i32 * Triangle = LookupPolygonEdges[IndexVoxel][TriangleIndex];

                            for (u32 EdgeIndex = 0;
                                    EdgeIndex < 3;
                                    ++EdgeIndex)
                            {
                                i32 Edge = Triangle[EdgeIndex];
                                u32 p00 = LookupEgdePoints[Edge][0];
                                u32 p01 = LookupEgdePoints[Edge][1];

                                v3 StartP0 = VoxelP + LookupVertexOffset[p00];
                                v3 FromStartToEnd = LookupVertexOffset[p01] - LookupVertexOffset[p00];
                                v3 EndP0 = StartP0 + FromStartToEnd;
                                v3 TestP0 = InterpolateEdgePosition(StartP0, EndP0, 1.0f,5);
                                // around the mesh center
                                Vertices[VerticeCount].P =  TestP0 + OffsetFromOrigin;
                                //Vertices[VerticeCount].P.z = -Vertices[VerticeCount].P.z;
                                Vertices[VerticeCount].P.x = -Vertices[VerticeCount].P.x;
                                Vertices[VerticeCount].UV.x = TestP0.x;
                                Vertices[VerticeCount].UV.y = 1.0f - TestP0.z;
                                //Logn("Vertice %i at " STRP,VerticeCount,FP(Vertices[VerticeCount].P));
                                ++VerticeCount;

#if 1
                                r32 DistToOrigen = LengthSqr(Vertices[VerticeCount].P);
                                if (DistToOrigen > MaxDist)
                                {
                                    MaxDist = DistToOrigen;
                                    VerticeMaxDist = VerticeCount - 1;
                                }
#endif
                            }

                            // Query 6 densities points. +/- to each direction
                            v3 Grad;
                            Grad.x = DensityAtP(VoxelP + V3(VoxelDim,0,0)) - DensityAtP(VoxelP + V3(-VoxelDim,0,0));
                            Grad.y = DensityAtP(VoxelP + V3(0,VoxelDim,0)) - DensityAtP(VoxelP + V3(0,-VoxelDim,0));
                            Grad.z = DensityAtP(VoxelP + V3(0,0,VoxelDim)) - DensityAtP(VoxelP + V3(0,0,-VoxelDim));

                            v3 N = -Normalize(Grad);

                            Vertices[VerticeCount - 3].N =  N;
                            Vertices[VerticeCount - 2].N =  N;
                            Vertices[VerticeCount - 1].N =  N;
                        }
                    }
                }
            }
        }
    }

    //Logn("Farthest vertice " STRP,FP(Vertices[VerticeMaxDist].P));

    TempArena->CurrentSize -= sizeof(vertex_point)*(MaxVerticesPerBlock - VerticeCount);

    return VerticeCount;
}

u32
FillBufferTestGround(memory_arena * TempArena, world_pos WorldP, world_pos Origen, u32 VoxelsPerAxis = 3)
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

#if 0
    // Avoid lossing precision
    // Must be power of 2 we doing & op
    i32 RinseAndRepeat = ((2 << 7) - 1);

    r32 WorldX = (r32)(WorldP.x & RinseAndRepeat);
    r32 WorldY = (r32)(WorldP.y & RinseAndRepeat);
    r32 WorldZ = (r32)(WorldP.z & RinseAndRepeat);

    r32 OrigenX = (r32)(Origen.x & RinseAndRepeat);
    r32 OrigenY = (r32)(Origen.y & RinseAndRepeat);
    r32 OrigenZ = (r32)(Origen.z & RinseAndRepeat);

    r32 OffsetX = -OrigenX;
    r32 OffsetY = -OrigenY;
    r32 OffsetZ = -OrigenZ;
#else
    r32 WorldX = (r32)WorldP.x;
    r32 WorldY = (r32)WorldP.y;
    r32 WorldZ = (r32)WorldP.z;
    r32 OffsetX = -(r32)Origen.x;
    r32 OffsetY = -(r32)Origen.y;
    r32 OffsetZ = -(r32)Origen.z;

#endif

#if 0
    if ((WorldP.x > Origen.x) && (WorldX < OrigenX))
    {
        OffsetX = (RinseAndRepeat - OrigenX);
    }
    else if ((WorldP.x < Origen.x) && (WorldX > OrigenX))
    {
        OffsetX = -(r32)RinseAndRepeat;
    }

    if ((WorldP.y > Origen.y) && (WorldY < OrigenY))
    {
        OffsetY = (RinseAndRepeat - OrigenY);
    }
    else if ((WorldP.y < Origen.y) && (WorldY > OrigenY))
    {
        OffsetY = -(r32)RinseAndRepeat;
    }

    if ((WorldP.z > Origen.z) && (WorldZ < OrigenZ))
    {
        OffsetZ = (RinseAndRepeat - OrigenZ);
    }
    else if ((WorldP.z < Origen.z) && (WorldZ > OrigenZ))
    {
        OffsetZ = -(r32)RinseAndRepeat;
    }
#endif

    v3 OffsetFromOrigin = V3(OffsetX,OffsetY,OffsetZ);

    // Center block as origin
    v3 P = V3(WorldX,WorldY,WorldZ) + V3(0.5f);

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
                    r32 vx = WorldX + VoxelXIndex * VoxelDim + HalfVoxelDim;
                    r32 vy = WorldY + VoxelYIndex * VoxelDim + HalfVoxelDim;
                    r32 vz = WorldZ + VoxelZIndex * VoxelDim + HalfVoxelDim;

                    v3 VoxelP = V3(vx,vy,vz);

                    u32 IndexVoxel = GetDensityIndex(vx,vy,vz,HalfVoxelDim);
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

                                v3 StartP0 = (VoxelP + GetIncrementForVertex(p00,HalfVoxelDim));
                                v3 FromStartToEnd = (GetIncrementForVertex(p01,HalfVoxelDim) - GetIncrementForVertex(p00,HalfVoxelDim));
                                v3 EndP0 = StartP0 + FromStartToEnd;
                                v3 TestP0 = InterpolateEdgePosition(StartP0, EndP0, 1.0f,5);
                                // around the mesh center
                                //Vertices[VerticeCount].P =  ((TestP0 + OffsetFromOrigin) - V3(0.5f)) * 2.0f;
                                Vertices[VerticeCount].P =  TestP0 + OffsetFromOrigin;
                                //Logn("Vertice %i at " STRP,VerticeCount,FP(Vertices[VerticeCount].P));
                                ++VerticeCount;

#if 1
                                r32 DistToOrigen = LengthSqr(Vertices[VerticeCount].P);
                                if (DistToOrigen > MaxDist)
                                {
                                    MaxDist = DistToOrigen;
                                    VerticeMaxDist = VerticeCount - 1;
                                }
#endif
                            }
                            v3 A = Vertices[VerticeCount - 3].P;
                            v3 B = Vertices[VerticeCount - 2].P;
                            v3 C = Vertices[VerticeCount - 1].P;

                            // Query 6 densities points. +/- to each direction
                            v3 Grad;
                            Grad.x = DensityAtP(VoxelP + V3(VoxelDim,0,0)) - DensityAtP(VoxelP + V3(-VoxelDim,0,0));
                            Grad.y = DensityAtP(VoxelP + V3(0,VoxelDim,0)) - DensityAtP(VoxelP + V3(0,-VoxelDim,0));
                            Grad.z = DensityAtP(VoxelP + V3(0,0,VoxelDim)) - DensityAtP(VoxelP + V3(0,0,-VoxelDim));

                            //v3 N = Cross((B - A),(C - A));
                            v3 N = Normalize(Grad);

                            Vertices[VerticeCount - 3].N =  N;
                            Vertices[VerticeCount - 2].N =  N;
                            Vertices[VerticeCount - 1].N =  N;
                        }
                    }
                }
            }
        }
    }

    //Logn("Farthest vertice " STRP,FP(Vertices[VerticeMaxDist].P));

    TempArena->CurrentSize -= sizeof(vertex_point)*(MaxVerticesPerBlock - VerticeCount);

    return VerticeCount;
}

//THREAD_WORK_HANDLER(GroundInBatches)
void
GroundInBatches(async_load_ground * Data)
{
    async_load_ground * WorkData = (async_load_ground *)Data;
    Assert(WorkData->ThreadArena);
    
    world * World                    = WorkData->World;
    mesh_group * MeshGroup           = WorkData->MeshGroup;
    memory_arena * TempArena         = &WorkData->ThreadArena->Arena;
    world_pos WorldP                 = WorkData->WorldP;
    u32 VoxelsPerAxis  = WorkData->VoxelsPerAxis;

    i32 MaxTileX = WorkData->MaxTileX;
    i32 MaxTileY = WorkData->MaxTileY;
    i32 MaxTileZ = WorkData->MaxTileZ;

    vertex_point * Vertices = PushArray(TempArena,0,vertex_point);

    BeginTempArena(TempArena,1);
    
    u32 TotalVertices = 0;

    // voxels are -z, +z
    //WorldP.z = - WorldP.z;
    //WorldP.z = WorldP.z * -1;

    for (i32 x = -(MaxTileX /2 ); x <= (MaxTileX /2 ); ++x)
    {
        for (i32 y = -(MaxTileY / 2); y <= (MaxTileY / 2); ++y)
        {
            for (i32 z = -(MaxTileZ / 2); z <= (MaxTileZ / 2); ++z)
            {
                // Ground generator coord z goes from - to + as you get farther along
                // which is the opposite we do in our world space
                world_pos P;
                P.x = WorldP.x + x;
                P.y = WorldP.y + y;
                P.z = WorldP.z + z;

                //u32 VerticesCount = FillBufferTestGround(TempArena, P,WorldP,VoxelsPerAxis);
                u32 VerticesCount = FillBufferTestGroundCorner(TempArena, P,WorldP,VoxelsPerAxis);

                TotalVertices += VerticesCount;
            }
        }
    }

    if (TotalVertices > 0)
    {
        i32 SizeVertexBuffer = sizeof(vertex_point) * TotalVertices;
        u32 VertexBufferBeginOffset = 0;

        if (MeshGroup->Meshes->VertexSize < 0)
        {
            //Assert(MeshGroup->Meshes->VertexSize >= SizeVertexBuffer);
            SizeVertexBuffer = minval(MeshGroup->Meshes->VertexSize, SizeVertexBuffer);
            VertexBufferBeginOffset = MeshGroup->Meshes->OffsetVertices;
        }
        else
        {
            VertexBufferBeginOffset = PushMeshSize(WorkData->VertexArena,SizeVertexBuffer,1);
        }


        RenderPushVertexData(Vertices,SizeVertexBuffer, VertexBufferBeginOffset); 

        MeshGroup->Loaded = true;
        MeshGroup->Meshes->VertexSize = SizeVertexBuffer;
        MeshGroup->Meshes->OffsetVertices = VertexBufferBeginOffset;
    }

    EndTempArena(TempArena,1);

}

u32
CalculateMeshSize(u32 VoxelsPerAxis)
{
    u32 MaxTrianglesPerVoxel = 5;
    u32 TotalVoxelsPerBlock = (VoxelsPerAxis * VoxelsPerAxis * VoxelsPerAxis);
    u32 MaxTrianglesPerBlock = TotalVoxelsPerBlock * MaxTrianglesPerVoxel;
    u32 MaxVerticesPerBlock = MaxTrianglesPerBlock * 3;

    u32 Result = MaxVerticesPerBlock * sizeof(vertex_point);

    return Result;
}

THREAD_WORK_HANDLER(LoadGround)
{
    async_load_ground * WorkData = (async_load_ground *)Data;
    Assert(WorkData->ThreadArena);
    
    world * World                    = WorkData->World;
    mesh_group * MeshGroup           = WorkData->MeshGroup;
    memory_arena * TempArena         = &WorkData->ThreadArena->Arena;
    world_pos WorldP                 = WorkData->WorldP;
    
    WorldP.z = -WorldP.z;

    u32 VoxelsPerAxis = WorkData->VoxelsPerAxis;

    mesh * Mesh = MeshGroup->Meshes;
    u32 VertexBufferBeginOffset = Mesh->OffsetVertices;

    vertex_point * VerticesBuffer = PushArray(TempArena,0,vertex_point);

    //Logn("Attemp to create ground at " STRWORLDP,FWORLDP(WorldP));
    u32 CountVertices = FillBufferTestGround(TempArena, WorldP, WorldP,VoxelsPerAxis);

    if (CountVertices > 0)
    {
        //Logn("Success at (" STRWORLDP ") total %i",FWORLDP(WorldP),CountVertices);
        u32 SizeVertexBuffer = sizeof(vertex_point) * CountVertices;

        Assert(CalculateMeshSize(VoxelsPerAxis) >= SizeVertexBuffer);

        Mesh->VertexSize = SizeVertexBuffer;

        RenderPushVertexData(VerticesBuffer,SizeVertexBuffer, VertexBufferBeginOffset); 
    }

    ThreadEndArena(WorkData->ThreadArena);

    COMPILER_DONOT_REORDER_BARRIER;
    MeshGroup->Loaded        = true;  // b32 Loaded;
    MeshGroup->LoadInProcess = false; // b32 LoadInProcess;
}

/*
 * Signals whether managed to get thread arena.
 * if it didn't, no work was done
 */
bool
TryLoadGround(game_memory * Memory,
              game_state * GameState,
              world_pos WorldP, 
              mesh_group * MeshGroup,
              u32 VoxelsPerAxis)
{
    b32 LoadSuccess = false;

    thread_memory_arena * ThreadArena = GetThreadArena(GameState);
    if (IS_NOT_NULL(ThreadArena))
    {
        MeshGroup->LoadInProcess = true;

        memory_arena * Arena = ThreadBeginArena(ThreadArena);

        async_load_ground * Data = PushStruct(Arena,async_load_ground);

        MeshGroup->TotalMeshObjects = 1;

        Assert(IS_NOT_NULL(MeshGroup->Meshes)); // should be pre-allocated on world initialization

        Data->World       = &GameState->World; // world * World;
        Data->ThreadArena = ThreadArena;       // thread_memory_arena * ThreadArena;
        Data->MeshGroup   = MeshGroup;         // mesh_group * MeshGroup;
        Data->WorldP      = WorldP;            // RECORD WorldP;
        Data->VoxelsPerAxis = VoxelsPerAxis;

        Memory->AddWorkToWorkQueue(Memory->RenderWorkQueue , LoadGround,Data);

        LoadSuccess = true;
    }

    return LoadSuccess;
}


world_pos
WorldToChunkP(world_pos WorldP)
{
    i32 DimX = 11;
    i32 DimY = 5;
    i32 DimZ = 11;

    i32 OrigenChunkX = (i32)floor((r32)WorldP.x / (r32)DimX);
    i32 OrigenChunkY = (i32)floor((r32)WorldP.y / (r32)DimY);
    i32 OrigenChunkZ = (i32)floor((r32)WorldP.z / (r32)DimZ);

    world_pos Result = WorldPosition(OrigenChunkX, OrigenChunkY,OrigenChunkZ);

    return Result;
}

u32
GroundChunkHashIndex(world * World, i32 X, i32 Y, i32 Z)
{
    u32 NumBuckets = World->HashGridGroundEntitiesSize;

    const i32 h1 = 0x8da6b343; // Large multiplicative constants;
    const i32 h2 = 0xd8163841; // here arbitrarily chosen primes
    const i32 h3 = 0xcb1ab31f;
    i32 n = h1 * X + h2 * Y + h3 * Z;
    n = n % NumBuckets;

    if (n < 0) n += NumBuckets;

    return (u32)n;
}

void 
GenerateGround(game_memory * Memory,game_state * GameState,world * World,world_pos OrigenP)
{
    START_CYCLE_COUNT(ground_generation);
    //OrigenP = WorldPosition(-2,0,-2);
    //OrigenP = WorldPosition(0,0,0);

    v3 GroundArea = World->GridCellDimInMeters * 12.0f;

    v3 MinCorner = OrigenP._Offset - GroundArea; 
    v3 MaxCorner = OrigenP._Offset + GroundArea; 

    world_pos MinCell = MapIntoCell(World, OrigenP, MinCorner);
    MinCell.y = -1;
    world_pos MaxCell = MapIntoCell(World, OrigenP, MaxCorner);
    MaxCell.y = 0;

    i32 DimX = 11;
    i32 DimY = 5;
    i32 DimZ = 11;

    //r32 GroundArtificialScale = 0.95f;
    r32 GroundArtificialScale = 1.00f;
    //v3 GroundScaleDim = V3((r32)DimX/2.0f, (r32)DimY/2.0f, (r32)DimZ/2.0f);
    v3 GroundScaleDim = V3(1.0f);
    //v3 GroundScaleDim = V3((r32)DimX, (r32)DimY, (r32)DimZ);
    v3 GroundScale = VectorMultiply(World->GridCellDimInMeters,GroundScaleDim) * GroundArtificialScale;

    memory_arena * TempArena = &GameState->TemporaryArena;

    OrigenP.y = 0;
    world_pos OrigenInChunkP = WorldToChunkP(OrigenP);
#if 1
    world_pos MinChunkP = WorldToChunkP(MinCell);
    world_pos MaxChunkP = WorldToChunkP(MaxCell);
#else
    world_pos MinChunkP = WorldPosition(0,0,0);
    world_pos MaxChunkP = WorldPosition(0,0,0);
#endif

    BeginTempArena(TempArena,1);

    entity ** GroundCanBeDiscarded = PushArray(TempArena,World->GroundEntityLimit,entity *);
    r32 * GroundCanBeDiscardedLengthSqr = PushArray(TempArena,World->GroundEntityLimit,r32);
    u32 GroundCanBeDiscardedCount = 0;

    // create list of ground too far
    for (u32 GroundEntityIndex = 0;
            GroundEntityIndex < World->GroundEntityCount;
            ++GroundEntityIndex)
    {
        entity * TestEntity = World->GroundEntities + GroundEntityIndex;
        world_pos TestChunkP = TestEntity->GroundChunkP;
        if (
                (TestChunkP.x < MinChunkP.x || TestChunkP.x > MaxChunkP.x) ||
                (TestChunkP.y < MinChunkP.y || TestChunkP.y > MaxChunkP.y) ||
                (TestChunkP.z < MinChunkP.z || TestChunkP.z > MaxChunkP.z)
           )
        {
            // measure absolute distance not chunk
            v3 GroundToOrigin = Substract(World,OrigenInChunkP,TestChunkP);
            r32 LengthSqrToCenter  = (r32)fabs(LengthSqr(GroundToOrigin));
            GroundCanBeDiscardedLengthSqr[GroundCanBeDiscardedCount] = LengthSqrToCenter;
            GroundCanBeDiscarded[GroundCanBeDiscardedCount] = TestEntity;
            GroundCanBeDiscardedCount += 1;
        }
    }

    b32 AnyGroundCanBeDiscarded = (GroundCanBeDiscardedCount > 0);
    b32 AnyEmptyGroundBucket = (World->GroundEntityCount < World->GroundEntityLimit);

    for (i32 Z = MinChunkP.z;
            (Z <= MaxChunkP.z) && (AnyGroundCanBeDiscarded || AnyEmptyGroundBucket);
            ++Z)
    {
        for (i32 Y = MinChunkP.y;
                (Y <= MaxChunkP.y) && (AnyGroundCanBeDiscarded || AnyEmptyGroundBucket);
                ++Y)
        {
            for (i32 X = MinChunkP.x;
                    (X <= MaxChunkP.x) && (AnyGroundCanBeDiscarded || AnyEmptyGroundBucket);
                    ++X)
            {
                world_pos WorldP = WorldPosition(X*DimX, Y*DimY, Z*DimZ);
                world_pos EntityWorldP = WorldPosition(X*DimX + DimX/2, Y*DimY + DimY/2, Z*DimZ + DimZ/2,-World->GridCellDimInMeters * 0.5f);
                world_pos ChunkP = WorldPosition(X, Y, Z);

                b32 GroundExists              = false;
                r32 FarthestGroundDistance    = 0;

                u32 HashIndex = GroundChunkHashIndex(World, X, Y , Z);
#if 1
                i32 FreeHashIndex = -1;
                for (entity * TestEntity = World->HashGroundEntities[HashIndex];
                        IS_NOT_NULL(TestEntity);
                        )
                {
                    if (World->HashGroundOccupancy[HashIndex] > 0)
                    {
                        world_pos TestChunkP = TestEntity->GroundChunkP;
                        if (TestChunkP.x == ChunkP.x &&
                                TestChunkP.y == ChunkP.y &&
                                TestChunkP.z == ChunkP.z)
                        {
                            // clean up table hash
                            if (FreeHashIndex >= 0)
                            {
                                World->HashGroundOccupancy[HashIndex] = 0;
                                World->HashGroundOccupancy[FreeHashIndex] = 1;
                                World->HashGroundEntities[FreeHashIndex] = World->HashGroundEntities[HashIndex];
                                HashIndex = FreeHashIndex;
                            }
                            GroundExists = true;
                            break;
                        }
                    }
                    else if (FreeHashIndex < 0)
                    {
                        // This is a free bucket unused we can fill
                        FreeHashIndex = (i32)HashIndex;
                    }
                    HashIndex = (HashIndex + 1) & (World->HashGridGroundEntitiesSize - 1);
                    TestEntity = World->HashGroundEntities[HashIndex];
                }
#else
                for (u32 GroundEntityIndex = 0;
                        GroundEntityIndex < World->GroundEntityCount;
                        ++GroundEntityIndex)
                {
                    entity * TestEntity = World->GroundEntities + GroundEntityIndex;
                    world_pos TestChunkP = TestEntity->GroundChunkP;
                    if (TestChunkP.x == ChunkP.x &&
                            TestChunkP.y == ChunkP.y &&
                            TestChunkP.z == ChunkP.z)
                    {
                        GroundExists = true;
                        break;
                    }
                }
#endif

                if (!GroundExists)
                {
                    u32 GroundIndex = World->GroundEntityCount;
                    mesh_group * GroundMeshGroup = 0;
                    entity * EntityGround = 0;

                    if (AnyEmptyGroundBucket)
                    {
                        GroundMeshGroup = GameState->GroundMeshGroup + GroundIndex;
                        EntityGround = World->GroundEntities + GroundIndex;
                    }
                    else if (GroundCanBeDiscardedCount > 0)
                    {
                        EntityGround = GroundCanBeDiscarded[--GroundCanBeDiscardedCount];
                        GroundMeshGroup = GameState->GroundMeshGroup + EntityGround->MeshID.ID;
                    }
#if 1
                    // TODO: this might change depending on the LOD
                    u32 VoxelsPerAxis = 3;
                    b32 WasLoaded = GroundMeshGroup->Loaded;

                    GroundMeshGroup->Loaded        = false;
                    GroundMeshGroup->LoadInProcess = false;
#if 0
                    u32 MaxMeshSize = CalculateMeshSize(VoxelsPerAxis);
                    u32 Align = RenderGetVertexMemAlign() - 1;
                    u32 MeshSize = (MaxMeshSize + Align) & ~Align;

                    // pre-allocate ground mesh size
                    if (GroundMeshGroup->Meshes->VertexSize == 0)
                    {
                        GroundMeshGroup->Meshes->VertexSize = MeshSize;
                        GroundMeshGroup->Meshes->OffsetVertices = PushMeshSize(&GameState->VertexArena,MeshSize,1);
                    }
                    else
                    {
                        //Assert(GroundMeshGroup->Meshes->VertexSize >= MeshSize);
                    }


                    // Ground is not necessary to be in screen
                    // Allow thread load based on capacity
                    b32 GroundLoaded = TryLoadGround(Memory,GameState,EntityWorldP, GroundMeshGroup,VoxelsPerAxis);

#else 
                    async_load_ground Data;
                    Data.World            = World; // world * World;
                    Data.ThreadArena      = GetThreadArena(GameState); // thread_memory_arena * ThreadArena;
                    Data.MeshGroup        = GroundMeshGroup; // mesh_group * MeshGroup;
                    Data.WorldP           = EntityWorldP; // RECORD   WorldP;
                    Data.MaxTileX         = DimX; // i32   MaxTileX;
                    Data.MaxTileY         = DimY; // i32   MaxTileY;
                    Data.MaxTileZ         = DimZ; // i32   MaxTileZ;
                    Data.VoxelsPerAxis    = VoxelsPerAxis; // u32   VoxelsPerAxis;
                    Data.GameState        = GameState; // game_state * GameState;
                    Data.VertexArena      = &GameState->Renderer.VertexArena;

                    GroundInBatches(&Data);
                    ThreadEndArena(Data.ThreadArena);
                    b32 GroundLoaded = true;
#endif
#else
                    b32 WasLoaded = GroundMeshGroup->Loaded;

                    GroundMeshGroup->Loaded        = false;
                    GroundMeshGroup->LoadInProcess = false;
                    //u32 TotalVertices = GroundInBatches(GameState, ChunkP, GroundMeshGroup, DimX, DimY,DimZ, 3);
                    vertex_point * Vertices = PushArray(TempArena,1,vertex_point);
                    u32 TotalVertices = CopyCubeToBuffer(Vertices, 0.5f);
                    u32 MeshSize = TotalVertices * sizeof(vertex_point);
                    u32 VertexOffset = PushMeshSize(&GameState->VertexArena,MeshSize,1);
                    RenderPushVertexData(Vertices,MeshSize, VertexOffset); 
                    GroundMeshGroup->Meshes->VertexSize = MeshSize;
                    GroundMeshGroup->Meshes->OffsetVertices = VertexOffset;
                    GroundMeshGroup->Loaded = true;
                    b32 GroundLoaded = true;
#endif

                    if (GroundLoaded)
                    {
                        // Once thread is confirm to do the work commit
                        //Logn("WorldP    X:%i Y:%i Z:%i",X,Y,Z);
                        //Logn("In Range  X:%i Y:%i Z:%i",ChunkDimX,ChunkDimY,ChunkDimZ);
                        //Logn("Ground load at " STRWORLDP, FWORLDP(EntityWorldP));

                        if (GroundIndex < World->GroundEntityLimit)
                        {
                            GameState->GroundMeshCount += 1;
                            World->GroundEntityCount += 1;
                        }
                        else
                        {
                            // Using ground too far instead
                            GroundIndex = EntityGround->MeshID.ID;
                            world_pos EntityDiscardedWP = EntityGround->GroundChunkP;
                            u32 HashIndexGroundDiscarded = 
                                GroundChunkHashIndex(World,
                                        EntityDiscardedWP.x,EntityDiscardedWP.y,EntityDiscardedWP.z);
                            entity * TestEntity = World->HashGroundEntities[HashIndexGroundDiscarded];
                            for (;IS_NOT_NULL(TestEntity);)
                            {
                                if (World->HashGroundOccupancy[HashIndexGroundDiscarded] > 0)
                                {
                                    world_pos TestChunkP = TestEntity->GroundChunkP;
                                    if (TestChunkP.x == EntityDiscardedWP.x &&
                                            TestChunkP.y == EntityDiscardedWP.y &&
                                            TestChunkP.z == EntityDiscardedWP.z)
                                    {
                                        break;
                                    }
                                }
                                HashIndexGroundDiscarded = (HashIndexGroundDiscarded + 1) & (World->HashGridGroundEntitiesSize - 1);
                                TestEntity = World->HashGroundEntities[HashIndexGroundDiscarded];
                            }
                            // we should have found it
                            Assert(TestEntity);
                            World->HashGroundOccupancy[HashIndexGroundDiscarded] = 0;
                        }

                        UpdateGroundEntity(EntityGround, EntityWorldP, ChunkP, GroundScale);

                        u32 FreeSlot = HashIndex;
                        for (;World->HashGroundOccupancy[FreeSlot] > 0;) 
                        { 
                            FreeSlot = (FreeSlot + 1) & (World->HashGridGroundEntitiesSize - 1);
                        }
                        World->HashGroundEntities[FreeSlot] = EntityGround;

                        if (World->HashGroundOccupancy[FreeSlot] == -1)
                        {
                            World->HashGridUsageCount += 1;
                        }

                        World->HashGroundOccupancy[FreeSlot] = 1;

                        EntityGround->MeshID.ID = GroundIndex;
                        EntityGround->ID.ID = GroundIndex;

                        v3 Color = V3(0.3f,0.8f,0.0f);
                        if ((ChunkP.x % 2) == 0)
                        {
                            if ((ChunkP.z % 2) == 0)
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
                            if ((ChunkP.z % 2) == 1)
                            {
                                Color = V3(1.0f,0,0);
                            }
                            else
                            {
                                Color = V3(0.0f,0,1.0f);
                            }
                        }

                        EntityGround->Color = Color;
                        EntityGround->MeshObjCount = 1;
                    }
                    else
                    {
                        GroundMeshGroup->Loaded = WasLoaded;
                    }

                }

                AnyGroundCanBeDiscarded = (GroundCanBeDiscardedCount > 0);
                AnyEmptyGroundBucket = (World->GroundEntityCount < World->GroundEntityLimit);
            }
        }
    }

#if 0
    r32 MaxGroundDistance = 0;

    for (u32 GroundEntityIndex = 0;
            GroundEntityIndex < World->GroundEntityCount;
            ++GroundEntityIndex)
    {
        entity * TestEntity = World->GroundEntities + GroundEntityIndex;
        world_pos TestP = TestEntity->WorldP;

        // measure absolute distance not chunk
        v3 GroundToOrigin = Substract(World,OrigenP,TestP);
        r32 LengthSqrToCenter  = (r32)fabs(LengthSqr(GroundToOrigin));
        if (LengthSqrToCenter > MaxGroundDistance)
        {
            MaxGroundDistance = LengthSqrToCenter;
        }
    }

    for (u32 GroundEntityIndex = 0;
            GroundEntityIndex < World->GroundEntityCount;
            ++GroundEntityIndex)
    {
        entity * TestEntity = World->GroundEntities + GroundEntityIndex;
        world_pos TestP = TestEntity->WorldP;
        v3 GroundToOrigin = Substract(World,OrigenP,TestP);
        r32 LengthSqrToCenter  = (r32)fabs(LengthSqr(GroundToOrigin));
        r32 ColorIntensity = LengthSqrToCenter * (1.0f / MaxGroundDistance);
        TestEntity->Color = V3(1.0f - ColorIntensity,0, ColorIntensity);
    }
#endif
    for (u32 GroundEntityIndex = 0;
            GroundEntityIndex < World->GroundEntityCount;
            ++GroundEntityIndex)
    {
        entity TestEntity = World->GroundEntities[GroundEntityIndex];
        
        TestEntity.Color = V3(1.0f,0.0f,0);
        TestEntity.Transform.LocalS = V3(1.0f);
        UpdateTransform(&TestEntity);
        PushDrawDebug(&GameState->Renderer,&TestEntity);
    }

    // re build hash table
    if (World->HashGridUsageCount > (0.5 * World->HashGridGroundEntitiesSize))
    {
        for (u32 Index = 0; Index < World->HashGridGroundEntitiesSize;++Index) 
        { 
            World->HashGroundOccupancy[Index] = -1;
            World->HashGroundEntities[Index] = 0;
        }

        World->HashGridUsageCount = 0;

        for (u32 GroundEntityIndex = 0;
                GroundEntityIndex < World->GroundEntityCount;
                ++GroundEntityIndex)
        {
            entity * TestEntity = World->GroundEntities + GroundEntityIndex;
            world_pos TestP = TestEntity->GroundChunkP;

            i32 HashIndex = GroundChunkHashIndex(World, TestP.x, TestP.y, TestP.z);

            for (;World->HashGroundOccupancy[HashIndex] > 0;) 
            { 
               HashIndex = (HashIndex + 1) & (World->HashGridGroundEntitiesSize - 1);
            }

            World->HashGroundEntities[HashIndex] = TestEntity;
            World->HashGroundOccupancy[HashIndex] = 1;
            World->HashGridUsageCount += 1;
        }
        Logn("Hash table for ground rebuilt (Usage %i)",World->HashGridUsageCount);
    }


    EndTempArena(TempArena,1);

    END_CYCLE_COUNT(ground_generation);
}
