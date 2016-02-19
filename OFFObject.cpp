#include "OFFObject.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "Window.h"
#include "math.h"
#include <sstream>
#include <fstream>

#include "Globals.h"

using namespace std;

#define deleteVector(__vect__) do {\
                                   iter = __vect__->begin();\
                                   end = __vect__->end();\
                                   while(iter != end) delete (*(iter++));\
                                   delete __vect__;\
                               } while(false)

OFFObject::OFFObject()
{
	this->vertices = new std::vector<Vector3*>();
	this->normals = new std::vector<Vector3*>();
	this->texcoords = new std::vector<Vector3*>();
	this->face_normals = new std::vector<Vector3*>();
	this->faces = new std::vector<_Face>();
	this->face_adjacency = new vector<vector<int>*>();

	this->edges = new vector<_Edge>();
	this->edge_adjacency = new vector<vector<int>*>();

	this->face_quadrics = new vector<Quadric>();
	this->vertex_quadrics = new vector<Quadric>();
}

OFFObject::OFFObject(string filename) {
	this->vertices = new std::vector<Vector3*>();
	this->normals = new std::vector<Vector3*>();
	this->texcoords = new std::vector<Vector3*>();
	this->faces = new std::vector<_Face>();
	this->face_adjacency = new vector<vector<int>*>(); 
	this->face_normals = new std::vector<Vector3*>();

	this->edges = new vector<_Edge>();
	this->edge_adjacency = new vector<vector<int>*>();

	this->face_quadrics = new vector<Quadric>();
	this->vertex_quadrics = new vector<Quadric>();

	parse(filename);
}


OFFObject::~OFFObject()
{
	vector<Vector3*>::iterator iter;
	vector<Vector3*>::iterator end;
	deleteVector(vertices);
	deleteVector(normals);
	deleteVector(texcoords);
	deleteVector(face_normals);

	delete face_adjacency;
	delete edge_adjacency;

	delete faces;
	delete edges;
	delete face_quadrics;
	delete vertex_quadrics;
}

void OFFObject::draw(DrawData& data) {
	material.apply();

	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glMultMatrixf(toWorld.ptr());

	glBegin(GL_TRIANGLES);

	//Loop through the faces
	//For each face:
	//  Look up the vertices, normals (if they exists), and texcoords (if they exist)
	//  Draw them as triplets:

	//      glTex(texcoords->at(face.ts[0]))
	//      glNorm(normals->at(face.ns[0]))
	//      glVert(vertices->at(face.vs[0]))
	//      Etc.
	//.
	for each(_Face f in *faces) {
		for (int i = 0; i < 3; i++) {
			Vector3 n = *normals->at(f.vs[i]);
			Vector3 v = *vertices->at(f.vs[i]);
			//glColor3f((n[0]+1)/2.0, (n[1]+1)/2.0, (n[2]+1)/2.0);
			float random = (double)rand() / RAND_MAX;
			glColor3f(random, random, random);
			glNormal3f(n[0], n[1], n[2]);
			glVertex3f(v[0], v[1], v[2]);
		}
	}

	glEnd();

	glPopMatrix();
}

void OFFObject::parse(string& filename) {
	std::ifstream infile(filename);
	std::string line;
	std::vector<std::string> tokens;
	std::string token;

	int vNum, fNum;
	int lnCount = 1;

	cout << "Strat parsing" << endl;

	while (getline(infile, line)) {
		tokens.clear();
		tokens = split(line, ' ', tokens);

		if (line.empty())
			continue;

		if (tokens.at(0).compare("OFF") == 0) {
			lnCount++;
			continue;
		}
		if (lnCount == 2) {
			vNum = stof(tokens.at(0));
			fNum = stof(tokens.at(0));
			lnCount++;
			continue;
		}
		if (lnCount <= vNum + 2) {
			float x = std::stof(tokens.at(0));
			float y = std::stof(tokens.at(1));
			float z = std::stof(tokens.at(2));
			vertices->push_back(new Vector3(x, y, z));
			face_adjacency->push_back(new vector<int>());
			edge_adjacency->push_back(new vector<int>());
			lnCount++;
			continue;
		}
		if (lnCount > vNum && tokens.at(0).compare("3") == 0) {
			_Face face;

			//Parse the face line
			face.vs[0] = stof(tokens.at(1));
			face.vs[1] = stof(tokens.at(2));
			face.vs[2] = stof(tokens.at(3));

			faces->push_back(face);
			lnCount++;
			continue;
		}
	}
	std::cout << "Done parsing." << std::endl;

	calc_face_normals();
	calc_face_adjacency();
	calc_vertex_normals();
	fundamental_quadrics();
	calc_vertex_quadrics();
	calc_edges();
	calc_edge_adjacency();
	calc_edge_costs();

	return;
}

void OFFObject::calc_face_normals() {
	cout << "Start calculating face normals" << endl;

	for each(_Face f in *faces) {
		Vector3 normal = (*vertices->at(f.vs[0]) - *vertices->at(f.vs[1])).cross(*vertices->at(f.vs[2]) - *vertices->at(f.vs[1]));
		face_normals->push_back(new Vector3(normal.normalize().negate()));
	}

	cout << "Done calculating face normals" << endl;
}

void OFFObject::calc_face_adjacency() {
	cout << "Start constructing face adjacency" << endl;

	for (int i = 0; i < faces->size(); i++) {
		_Face f = faces->at(i);
		face_adjacency->at(f.vs[0])->push_back(i);
		face_adjacency->at(f.vs[1])->push_back(i); 
		face_adjacency->at(f.vs[2])->push_back(i);
	}

	cout << "Done constructing face adjacency" << endl;
}

void OFFObject::calc_vertex_normals() {
	cout << "Start calculating vertex normals" << endl;

	vector<int>::iterator it;
	for (int i = 0; i < vertices->size(); i++) {
		Vector3 normal = Vector3(0, 0, 0);
		for (it = face_adjacency->at(i)->begin(); it != face_adjacency->at(i)->end(); it++) {
			normal = normal + *face_normals->at(*it);
		}
		normals->push_back(new Vector3(normal.normalize()));
	}

	cout << "Done calculating vertex normals" << endl;
}

void OFFObject::calc_edges() {
	cout << "Building edges" << endl;

	unordered_map<size_t,_Edge> map;
	_Edge e0, e1, e2;

	for each(_Face f in *faces) {
		e0.v0 = std::fminf(f.vs[0], f.vs[1]);
		e0.v1 = std::fmaxf(f.vs[0], f.vs[1]);
		e1.v0 = std::fminf(f.vs[1], f.vs[2]);
		e1.v1 = std::fmaxf(f.vs[1], f.vs[2]);
		e2.v0 = std::fminf(f.vs[0], f.vs[2]);
		e2.v1 = std::fmaxf(f.vs[0], f.vs[2]);

		map.insert(make_pair(hash<_Edge>{}(e0), e0));
		map.insert(make_pair(hash<_Edge>{}(e1), e1));
		map.insert(make_pair(hash<_Edge>{}(e2), e2));
	}

	unordered_map<rsize_t, _Edge>::iterator it;
	for (it = map.begin(); it != map.end(); it++) {
		edges->push_back(it->second);
	}

	cout << "Done building edges" << endl;
}

void OFFObject::calc_edge_adjacency() {
	for (int i = 0; i < edges->size(); i++) {
		_Edge e = edges->at(i);
		edge_adjacency->at(e.v0)->push_back(i);
		edge_adjacency->at(e.v1)->push_back(i);
	}
}

void OFFObject::fundamental_quadrics() {
	for (int i = 0; i < faces->size(); i++) {
		Vector3 a = *vertices->at(faces->at(i).vs[0]);
		Vector3 n = *face_normals->at(i);
		float d = -(a.dot(n));
		face_quadrics->push_back(calc_quadric(n[0], n[1], n[2], d));
	}
}

Quadric OFFObject::calc_quadric(float a, float b, float c, float d) {
	return Quadric{ a*a, a*b, a*c, a*d,
		b*b, b*c, b*d,
		c*c, c*d,
		d*d };
}

void OFFObject::calc_vertex_quadrics() {
	for (int i = 0; i < vertices->size(); i++) {
		vector<int>::iterator it;
		Quadric q;
		for (it = face_adjacency->at(i)->begin(); it < face_adjacency->at(i)->end(); it++) {
			q = q.add(face_quadrics->at(*it));
		}
		vertex_quadrics->push_back(q);
	}
}

void OFFObject::calc_edge_costs() {
	vector<_Edge>::iterator it;
	for (it = edges->begin(); it != edges->end(); it++) {
		(*it).cost = edge_cost(*it);
	}
}

float OFFObject::edge_cost(_Edge e) {
	Vector3 v = (*vertices->at(e.v0) + *vertices->at(e.v1))*0.5;
	Quadric q1 = vertex_quadrics->at(e.v0);
	Quadric q2 = vertex_quadrics->at(e.v1);
	return q1.error(v[0], v[1], v[2]) + q2.error(v[0], v[1], v[2]);
}

std::vector<std::string>& OFFObject::split(const std::string &s, char delim, std::vector<std::string> &elems)
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim))
	{
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> OFFObject::split(const std::string &s, char delim)
{
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

void OFFObject::update(UpdateData& data)
{
	//
}