#ifndef CSE190_OFFObject_h
#define CSE190_OFFObject_h

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include "Vector3.h"
#include "Drawable.h"
using namespace std;


struct _Face
{
	int vs[3];
};

struct _Edge {
	int v0;
	int v1;
};

namespace std {
	template<> struct hash<_Edge> {
		typedef _Edge argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& s) const
		{
			result_type const h1(std::hash<int>()(s.v0));
			result_type const h2(std::hash<int>()(s.v1));
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

	std::unordered_map<size_t, _Edge>* edges;

	void parse(string&);
	void calc_face_normals();
	void calc_face_adjacency();
	void calc_vertex_normals();
	void calc_edges();

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
