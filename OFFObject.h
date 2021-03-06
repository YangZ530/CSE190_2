#ifndef CSE190_OFFObject_h
#define CSE190_OFFObject_h

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <stack>
#include <algorithm>
#include "Vector3.h"
#include "Drawable.h"
using namespace std;

struct _Face{
	int vs[3];
	bool enable;
};

struct _Edge {
	//int v0;
	//int v1;
	int vs[2];
	float cost;
	bool enable;
};

struct Record{
	_Edge oldEdge;
	int newVertex;
	std::vector<int> faces;
	//Record(_Edge edge, int v) : oldEdge(edge), newVertex(v)
	//{}
};

struct Quadric {
	float q11, q12, q13, q14, 
		q22, q23, q24, q33, 
		q34, q44;
	Quadric() :q11(0), q12(0), q13(0), q14(0),
		q22(0), q23(0), q24(0),
		q33(0), q34(0), q44(0) {}
	Quadric(float aa, float ab, float ac, float ad, float bb,
		float bc, float bd, float cc, float cd, float dd) :
		q11(aa), q12(ab), q13(ac), q14(ad),
		q22(bb), q23(bc), q24(bd),
		q33(cc), q34(cd), q44(dd) {}
	Quadric add(Quadric q) {
		return Quadric(q11 + q.q11, q12 + q.q12, q13 + q.q13, q14 + q.q14,
			q22 + q.q22, q23 + q.q23, q24 + q.q24, q33 + q.q33, q34 + q.q34, q44 + q.q44);
	}
	float error(float x, float y, float z) {
		return  q11*x*x + q12 * x * y *2.0 + q13*x*z*2.0 + q14*x * 2 +
			q22*y*y + q23*y*z*2.0 + q24*2.0 +
			q33*z*z + q34*z*2.0 + q44;
	}
};

namespace std {
	template<> struct hash<_Edge> {
		typedef _Edge argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& s) const
		{
			result_type const h1(std::hash<int>()(s.vs[0]));
			result_type const h2(std::hash<int>()(s.vs[1]));
			return h1 ^ (h2 << 1); // or use boost::hash_combine
		}
	};
}

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

	std::vector<Quadric>* face_quadrics;
	std::vector<Quadric>* vertex_quadrics;

	std::vector<_Edge>* edges;
	std::vector<vector<int>*>* edge_adjacency;

	std::stack<Record> records;

	bool geomorph;
	bool simplifying;
	bool progressing;
	Vector3 interpolatedV0;
	Vector3 interpolatedV1;
	Vector3 interpolatedV;

	void parse(string&);
	void calc_face_normals();
	void calc_face_adjacency();
	void calc_vertex_normals();
	void calc_edges();
	void calc_edge_adjacency();
	Quadric calc_quadric(float a, float b, float c, float d);

	bool interpolate(_Edge, bool);
	void collapse(_Edge);
	void restore(Record);
	void calc_edge_costs();
	float edge_cost(_Edge);

	//Quadric calc_quadric(float, float, float, float);
	std::vector<std::string>& split(const std::string&, char, std::vector<std::string>&);
	std::vector<std::string> split(const std::string&, char);

	void fundamental_quadrics();
	Quadric calc_face_quadrics(int);
	void vertices_quadrics();
	Quadric calc_vertex_quadrics(int);
	void calc_all_quadrics();
	Quadric vertex_quadric(int);

	_Edge nextEdge();

	float randColor();

public:
	OFFObject();
	OFFObject(string);
	virtual ~OFFObject();

	virtual void draw(DrawData&);
	virtual void update(UpdateData&);
	void meshSimplify();
	void meshSimplify(int a, int b);
	void progressiveMesh();
};
#endif
