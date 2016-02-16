#ifndef CSE190_OFFObject_h
#define CSE190_OFFObject_h

#include <iostream>
#include <vector>
#include <string>
#include "Vector3.h"
#include "Drawable.h"
using namespace std;

struct _Face
{
	int vs[3];
};

class OFFObject :
	public Drawable
{
protected:
	std::vector<Vector3*>* vertices;
	std::vector<Vector3*>* normals;
	std::vector<Vector3*>* texcoords;
	std::vector<_Face>* faces;
	std::vector<Vector3*>* face_normals;
	std::vector<vector<int>*>* face_adjacency;

	void parse(string&);
	void calc_face_normals();
	void calc_face_adjacency();
	void calc_vertex_normals();

	std::vector<std::string>& split(const std::string&, char, std::vector<std::string>&);
	std::vector<std::string> split(const std::string&, char);

public:
	OFFObject();
	OFFObject(string);
	virtual ~OFFObject();

	virtual void draw(DrawData&);
	virtual void update(UpdateData&);
};
#endif
