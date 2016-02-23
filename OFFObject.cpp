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

#define V0 -1
#define V1 -2

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

	geomorph = true;
	simplifying = false;
	progressing = false;
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

	geomorph = true;
	simplifying = false;
	progressing = false;

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
		if (f.enable)
			for (int i = 0; i < 3; i++) {
				Vector3 n = *normals->at(f.vs[i]);
				Vector3 v = *vertices->at(f.vs[i]);
				glColor3f(randColor(), randColor(), randColor());
				glNormal3f(n[0], n[1], n[2]);
				glVertex3f(v[0], v[1], v[2]);
			}
	}

	glEnd();

	glPopMatrix();
}


/**
* 1. create new vertext
* 2. degenerate two faces
* 3. update adjacent faces
* 4. update adjacent edges
*/
void OFFObject::collapse(_Edge edge) {

	int v = 0;
	int v0 = edge.vs[0];
	int v1 = edge.vs[1];
	Vector3 vector = (*vertices->at(v0) + *vertices->at(v1)).scale(0.5f);
	Record record;

	cout << "v0 = " << v0 << endl;
	cout << "v1 = " << v1 << endl;

	// create new vertex
	vertices->push_back(new Vector3(vector));
	v = vertices->size() - 1;

	face_adjacency->push_back(new std::vector<int>());
	edge_adjacency->push_back(new std::vector<int>());

	// degenerate one/two faces	
	for each (int faceID in *face_adjacency->at(v0))
	{
		_Face * face = &faces->at(faceID);
		for (int i = 0; i < 3; i++)
			if (face->vs[i] == v1)
			{
				face->enable = false;
				record.faces.push_back(faceID);
			}
	}
	// update faces
	for each (int faceID in *face_adjacency -> at(v0))
	{
		_Face * face = &faces->at(faceID);
		if (!face->enable) continue;
		for (int i = 0; i < 3; i++)
			if (face->vs[i] == v0) // update adjacent faces
				face->vs[i] = v;

		Vector3 normal = (*vertices->at(face->vs[0]) - *vertices->at(face->vs[1])).cross(*vertices->at(face->vs[2]) - *vertices->at(face->vs[1]));
		*face_normals->at(faceID) = normal; // update face normal
		//face_quadrics->at(faceID) = calc_face_quadrics(faceID); // update face quadrics
		face_adjacency->at(v)->push_back(faceID);		
	}
	for each (int faceID in *face_adjacency -> at(v1))
	{
		_Face * face = &faces->at(faceID);
		if (!face->enable) continue;
		for (int i = 0; i < 3; i++) // update adjacent faces
			if (face->vs[i] == v1)
				face->vs[i] = v;

		Vector3 normal = (*vertices->at(face->vs[0]) - *vertices->at(face->vs[1])).cross(*vertices->at(face->vs[2]) - *vertices->at(face->vs[1]));
		*face_normals->at(faceID) = normal; // update face normal
		//face_quadrics->at(faceID) = calc_face_quadrics(faceID); // update face quadrics
		face_adjacency->at(v)->push_back(faceID);
	}

	//vertex_quadrics->push_back(calc_vertex_quadrics(v)); // get new vertex quadrics
	vertex_quadrics->push_back(vertex_quadrics->at(v0).add(vertex_quadrics->at(v1))); // get new vertex quadrics

	// update edges
	
	for each (int edgeID in *edge_adjacency -> at(v0))
	{
		_Edge * edge = &edges->at(edgeID);
		for (int i = 0; i < 2; i++)
			if (edge->vs[i] == v1) // degenerate two edges	
				edge->enable = false;			
	}
	for each (int edgeID in *edge_adjacency -> at(v0))
	{
		_Edge * edge = &edges->at(edgeID);
		if (!edge->enable) continue;
		for (int i = 0; i < 2; i++)
			if (edge->vs[i] == v0) // update adjacent edges
				edge->vs[i] = v;
			//else
				//vertex_quadrics->at(edge->vs[i]) = calc_vertex_quadrics(edge->vs[i]);
			if (edge->enable)
			{
				edge->cost = edge_cost(*edge);
				edge_adjacency->at(v)->push_back(edgeID);				
			}
	}
	for each (int edgeID in *edge_adjacency -> at(v1))
	{
		_Edge * edge = &edges->at(edgeID);
		if (!edge->enable) continue;
		for (int i = 0; i < 2; i++)
			if (edge->vs[i] == v1) // update adjacent edges
				edge->vs[i] = v;
			//else
				//vertex_quadrics->at(edge->vs[i]) = calc_vertex_quadrics(edge->vs[i]);
		if (edge->enable)
		{
			edge->cost = edge_cost(*edge);
			edge_adjacency->at(v)->push_back(edgeID);
		}
	}

	// update vertex normal
	Vector3 normal = Vector3(0, 0, 0);
	for each (int faceID in *face_adjacency -> at(v))
	{
		normal = normal + *face_normals->at(faceID);
	}
	normals->push_back(new Vector3(normal.normalize()));

	record.newVertex = v;
	record.oldEdge = edge;
	records.push(record);
}

/**
* 1. regenerate two faces
* 2. update adjacent faces
* 3. update adjacent edges
*/
void OFFObject::restore(Record record) {
	int v0 = record.oldEdge.vs[0];
	int v1 = record.oldEdge.vs[1];
	int v = record.newVertex;

	for each (int faceID in *face_adjacency -> at(v0))
	{
		_Face * face = &faces->at(faceID);
		if (!face->enable) continue;
		for (int i = 0; i < 3; i++)
		{
			if (face->vs[i] == v) // update adjacent faces
			{
				face->vs[i] = v0;
			}
		}
		Vector3 normal = (*vertices->at(face->vs[0]) - *vertices->at(face->vs[1])).cross(*vertices->at(face->vs[2]) - *vertices->at(face->vs[1]));
		*face_normals->at(faceID) = normal; // update face normal
		//face_quadrics->at(faceID) = calc_face_quadrics(faceID); // update face quadrics
	}
	for each (int faceID in *face_adjacency -> at(v1))
	{
		_Face * face = &faces->at(faceID);
		if (!face->enable) continue;
		for (int i = 0; i < 3; i++) // update adjacent faces
		{
			if (face->vs[i] == v) // update adjacent faces
			{
				face->vs[i] = v1;
			}
		}
		Vector3 normal = (*vertices->at(face->vs[0]) - *vertices->at(face->vs[1])).cross(*vertices->at(face->vs[2]) - *vertices->at(face->vs[1]));
		*face_normals->at(faceID) = normal; // update face normal
		//face_quadrics->at(faceID) = calc_face_quadrics(faceID); // update face quadrics
	}

	//regenerate faces
	for each (int faceID in record.faces)
	{
		_Face * face = &faces->at(faceID);
		face->enable = true; // regenerate faces
	}

	// update edges
	for each (int edgeID in *edge_adjacency -> at(v0))
	{
		_Edge * edge = &edges->at(edgeID);
		for (int i = 0; i < 2; i++)
		{
			edge->enable = true;
			if (edge->vs[i] == v) // update adjacent edges
			{
				edge->vs[i] = v0;
			}
			edge->cost = edge_cost(*edge);
		}
	}
	for each (int edgeID in *edge_adjacency -> at(v1))
	{
		_Edge * edge = &edges->at(edgeID);
		for (int i = 0; i < 2; i++)
		{
			edge->enable = true;
			if (edge->vs[i] == v) // update adjacent edges
			{
				edge->vs[i] = v1;
			}
			edge->cost = edge_cost(*edge);
		}
	}
}

/**
* Simplifies the mesh for one iteration
*/
void OFFObject::meshSimplify(int a, int b)
{
	_Edge edge = { a, b };
	collapse(edge);
}

/**
* Simplifies the mesh for one iteration
*/
void OFFObject::meshSimplify()
{
	/*
	for (int i = 0; i < edges->size(); i++)
	if (edges->at(i).enable)
	{
	collapse(edges->at(i));
	return;
	}
	*/
	//for (int i = 0; i < 10; i++)
	if (progressing) return;

	if (geomorph)
	{
		if (!simplifying)
		{
			interpolatedV0 = *vertices->at(nextEdge().vs[0]);
			interpolatedV1 = *vertices->at(nextEdge().vs[1]);
			simplifying = true;
		}
		geomorph = interpolate(nextEdge(), true);
	}
	else
	{
		*vertices->at(nextEdge().vs[0]) = interpolatedV0;
		*vertices->at(nextEdge().vs[1]) = interpolatedV1;
		collapse(nextEdge());
		geomorph = true;
		simplifying = false;
	}
}

void OFFObject::progressiveMesh()
{
	//for (int i = 0; i < 10; i++)
	if (simplifying || records.empty()) return;

	if (geomorph)
	{
		if (!progressing)
		{
			//interpolatedV = *vertices->at(records.top().newVertex);
			restore(records.top());
			interpolatedV0 = *vertices->at(records.top().oldEdge.vs[0]);
			interpolatedV1 = *vertices->at(records.top().oldEdge.vs[1]);
			*vertices->at(records.top().oldEdge.vs[0]) = *vertices->at(records.top().newVertex);
			*vertices->at(records.top().oldEdge.vs[1]) = *vertices->at(records.top().newVertex);
			progressing = true;
		}
		geomorph = interpolate(records.top().oldEdge, false);
	}
	else
	{
		*vertices->at(records.top().oldEdge.vs[0]) = interpolatedV0;
		*vertices->at(records.top().oldEdge.vs[1]) = interpolatedV1;
		vertices->pop_back();
		face_adjacency->pop_back();
		edge_adjacency->pop_back();
		records.pop();
		geomorph = true;
		progressing = false;
	}		

		/*		
		
		if (geomorph)
		{
			if (!simplifying)
			{
				interpolatedV0 = *vertices->at(nextEdge().vs[0]);
				interpolatedV1 = *vertices->at(nextEdge().vs[1]);
				simplifying = true;
			}
			geomorph = interpolate(nextEdge());
		}
		else
		{
			*vertices->at(nextEdge().vs[0]) = interpolatedV0;
			*vertices->at(nextEdge().vs[1]) = interpolatedV1;
			collapse(nextEdge());
			geomorph = true;
			simplifying = false;
		}
		*/
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
	vertices_quadrics();
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

	unordered_map<size_t, _Edge> map;
	_Edge e0, e1, e2;

	for each(_Face f in *faces) {
		e0.vs[0] = std::fminf(f.vs[0], f.vs[1]);
		e0.vs[1] = std::fmaxf(f.vs[0], f.vs[1]);
		e1.vs[0] = std::fminf(f.vs[1], f.vs[2]);
		e1.vs[1] = std::fmaxf(f.vs[1], f.vs[2]);
		e2.vs[0] = std::fminf(f.vs[0], f.vs[2]);
		e2.vs[1] = std::fmaxf(f.vs[0], f.vs[2]);

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
		edge_adjacency->at(e.vs[0])->push_back(i);
		edge_adjacency->at(e.vs[1])->push_back(i);
	}
}

void OFFObject::fundamental_quadrics() {
	for (int i = 0; i < faces->size(); i++) {
		face_quadrics->push_back(calc_face_quadrics(i));
	}
}

Quadric OFFObject::calc_face_quadrics(int i){
	Vector3 a = *vertices->at(faces->at(i).vs[0]);
	Vector3 n = *face_normals->at(i);
	float d = -(a.dot(n));
	return calc_quadric(n[0], n[1], n[2], d);
}

Quadric OFFObject::calc_quadric(float a, float b, float c, float d) {
	return Quadric{ a*a, a*b, a*c, a*d,
		b*b, b*c, b*d,
		c*c, c*d,
		d*d };
}

void OFFObject::vertices_quadrics() {
	for (int i = 0; i < vertices->size(); i++) {
		//vertex_quadrics->push_back(calc_vertex_quadrics(i));
		vertex_quadrics->push_back(vertex_quadric(i));
	}
}

void OFFObject::calc_all_quadrics() {
	for (int i = 0; i < vertices->size(); i++) {
		Quadric q = vertex_quadric(i);
		vertex_quadrics->push_back(q);
	}
}

Quadric OFFObject::vertex_quadric(int i) {
	vector<int>::iterator it;
	Vector3 a = *vertices->at(i);
	Quadric q = Quadric();
	for (it = face_adjacency->at(i)->begin(); it != face_adjacency->at(i)->end(); it++) {
		Vector3 n = *face_normals->at(*it);
		float d = -(a.dot(n));
		q = q.add(calc_quadric(n[0], n[1], n[2], d));
	}
	return q;
}

Quadric OFFObject::calc_vertex_quadrics(int i) {
		vector<int>::iterator it;
		Quadric q;
		for (it = face_adjacency->at(i)->begin(); it < face_adjacency->at(i)->end(); it++) {
			q = q.add(face_quadrics->at(*it));
		}
		return q;
}

void OFFObject::calc_edge_costs() {
	vector<_Edge>::iterator it;
	for (it = edges->begin(); it != edges->end(); it++) {
		(*it).cost = edge_cost(*it);
	}
}

float OFFObject::edge_cost(_Edge e) {
	Vector3 v = (*vertices->at(e.vs[0]) + *vertices->at(e.vs[1]))*0.5;
	Quadric q1 = vertex_quadrics->at(e.vs[0]);
	Quadric q2 = vertex_quadrics->at(e.vs[1]);
	return q1.error(v[0], v[1], v[2]) + q2.error(v[0], v[1], v[2]);
}

_Edge OFFObject::nextEdge(){
	float minCost = 1000.0f;
	int minID = 0;
	for (int i = 0; i < edges->size(); i++)
	{
		_Edge edge = edges->at(i);
		if (!edge.enable) continue;
		if (minCost > edge.cost)
		{
			minCost = edge.cost;
			minID = i;
		}
		if (edge.cost < 0)
			cout << "negative, cost = " << edge.cost << endl;
	}
	_Edge edge = edges->at(minID);
	return edge;
}

bool OFFObject::interpolate(_Edge edge, bool simplify){
	if (simplify)
	{
		int v0 = edge.vs[0];
		int v1 = edge.vs[1];
		Vector3 middle = (*vertices->at(v0) + *vertices->at(v1)).scale(0.5f);
		*vertices->at(v0) = (*vertices->at(v0) + middle).scale(0.5f);
		*vertices->at(v1) = (*vertices->at(v1) + middle).scale(0.5f);
		return (*vertices->at(v1) - *vertices->at(v0)).magnitude() > 1;
	}
	else
	{
		int v0 = edge.vs[0];
		int v1 = edge.vs[1];
		*vertices->at(v0) = (*vertices->at(v0) + interpolatedV0).scale(0.5f);
		*vertices->at(v1) = (*vertices->at(v1) + interpolatedV1).scale(0.5f);
		return (*vertices->at(v0) - interpolatedV0).magnitude() > 1;
	}
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

float OFFObject::randColor()
{
	return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

void OFFObject::update(UpdateData& data)
{
	//
}