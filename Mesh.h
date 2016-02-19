#ifndef CSE190_Mesh_h
#define CSE190_Mesh_h

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include "Vector3.h"

#include "Drawable.h"
class Mesh :
	public Drawable
{
public:
	Mesh();
	virtual ~Mesh();
};

#endif
