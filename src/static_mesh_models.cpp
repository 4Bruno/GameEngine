const char * MeshPaths[4] = {
	"assets\\cube_triangles.obj",
	"assets\\human_male_triangles.obj",
	"assets\\palm_tree.obj",
	"assets\\quad.obj"
};
const u32 MeshSizes[4] = {
	36 * sizeof(vertex_point),
	1968 * sizeof(vertex_point),
	576 * sizeof(vertex_point),
	3 * sizeof(vertex_point)
};
const u32 MeshObjects[4] = {
	1,
	1,
	7,
	1
};