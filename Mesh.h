#ifndef CSE190_Mesh_h
#define CSE190_Mesh_h

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include "Vector3.h"

#include "Drawable.h"

using namespace std;

struct Vertex {
	Vector3 vertex;
	Vector3 normal;
	vector<int> face_adj;
	vector<int> edge_adj;
};

struct Face {

};
class Mesh :
	public Drawable
{
public:
	Mesh();
	virtual ~Mesh();
};

#endif
