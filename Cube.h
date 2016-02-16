#ifndef CSE167_Cube_h
#define CSE167_Cube_h

#include "Drawable.h"

class Cube : public Drawable
{
    
public:
    
    float size;
    
    Cube(float);
    virtual ~Cube(void);
    
    virtual void draw(DrawData&);
    virtual void update(UpdateData&);
    
    void spin(float);
	void xMove(float);
	void yMove(float);
	void zMove(float);
	void zRotate(float);
	void trans(void);
	void scale(float);
	Matrix4 getToWorld(void);
};

#endif

