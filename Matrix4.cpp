#include <math.h>
#include <iostream>
#include <iomanip>
#include <cstring>
#include "Matrix4.h"
#include "Vector4.h"
#include "Vector3.h"

Matrix4::Matrix4()
{
    std::memset(m, 0, sizeof(m));
}

Matrix4::Matrix4(
                 float m00, float m01, float m02, float m03,
                 float m10, float m11, float m12, float m13,
                 float m20, float m21, float m22, float m23,
                 float m30, float m31, float m32, float m33 )
{
    this->set(m00, m01, m02, m03,
              m10, m11, m12, m13,
              m20, m21, m22, m23,
              m30, m31, m32, m33);
}

void Matrix4::set(float m00, float m01, float m02, float m03,
                  float m10, float m11, float m12, float m13,
                  float m20, float m21, float m22, float m23,
                  float m30, float m31, float m32, float m33)
{
    m[0][0] = m00;
    m[0][1] = m01;
    m[0][2] = m02;
    m[0][3] = m03;
    m[1][0] = m10;
    m[1][1] = m11;
    m[1][2] = m12;
    m[1][3] = m13;
    m[2][0] = m20;
    m[2][1] = m21;
    m[2][2] = m22;
    m[2][3] = m23;
    m[3][0] = m30;
    m[3][1] = m31;
    m[3][2] = m32;
    m[3][3] = m33;
}

float Matrix4::get(int vector,int element)
{
    return m[vector][element];
}

Matrix4& Matrix4::operator=(Matrix4 a)
{
    std::memcpy(m, a.m, sizeof(m));
    return *this;
}

float* Matrix4::ptr()
{
    return &m[0][0];
}

void Matrix4::identity()
{
    static const float ident[4][4]={{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}};
    std::memcpy(m, ident, sizeof(m));
}

Matrix4 Matrix4::multiply(Matrix4 a)
{
    Matrix4 b;
    
    //The current implementation below is not very efficient:
    //It allocates an additional 8 vectors on the stack and calls their constructors
    //It also calls off to additional functions (which create even more vectors!)
    //Which results in a lot of time being wasted on memory operations
    //This is bad, really bad, ultra bad D:
    
    //Implement a more performant Matrix * Matrix multiplication
    //Hint: Loops!
    //Hint for the ambitious: SIMD!
	/*
    Vector4 row1(m[0][0], m[1][0], m[2][0], m[3][0]);
    Vector4 row2(m[0][1], m[1][1], m[2][1], m[3][1]);
    Vector4 row3(m[0][2], m[1][2], m[2][2], m[3][2]);
    Vector4 row4(m[0][3], m[1][3], m[2][3], m[3][3]);
    
    Vector4 col1(a.m[0][0], a.m[0][1], a.m[0][2], a.m[0][3]);
    Vector4 col2(a.m[1][0], a.m[1][1], a.m[1][2], a.m[1][3]);
    Vector4 col3(a.m[2][0], a.m[2][1], a.m[2][2], a.m[2][3]);
    Vector4 col4(a.m[3][0], a.m[3][1], a.m[3][2], a.m[3][3]);

    b.set(row1.dot(col1), row2.dot(col1), row3.dot(col1), row4.dot(col1),
             row1.dot(col2), row2.dot(col2), row3.dot(col2), row4.dot(col2),
             row1.dot(col3), row2.dot(col3), row3.dot(col3), row4.dot(col3),
             row1.dot(col4), row2.dot(col4), row3.dot(col4), row4.dot(col4) );

    return b;
	*/
	
	// a for loop interpretation of the mathematics formula
	// at least saves mem
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			for (int k = 0; k < 4; k++)
				b.m[j][i] += m[k][i] * a.get(j,k);
		}
	}
	return b;
	
}

Matrix4 Matrix4::operator * (Matrix4 a)
{
    return multiply(a);
}

Vector4 Matrix4::multiply(Vector4 a)
{
    Vector4 b;
    
    //Implement Matrix * Vector4 multiplication
	// formula
	for (int i = 0; i < 4; i++){
		for (int k = 0; k < 4; k++)
			b[i] += m[k][i] * a[k];
	}
    
    return b;
}

Vector4 Matrix4::operator * (Vector4 a)
{
    return multiply(a);
}

Vector3 Matrix4::multiply(Vector3 a)
{
    Vector3 b;
    
    //Implement Matrix * Vector3 multiplication
    //Assume the 4th component is 0
	Vector4 c = multiply(Vector4(a[0], a[1], a[2], 0));
	b = Vector3(c[0], c[1], c[2]);
    
    return b;
}

Vector3 Matrix4::operator * (Vector3 a)
{
    return multiply(a);
}

Matrix4 Matrix4::makeRotateX(float angle)
{
    identity();
    
    //Configure this matrix to be a rotation about the X-Axis by 'angle' radians
	m[1][1] = cos(angle);
	m[1][2] = sin(angle);
	m[2][1] = -sin(angle);
	m[2][2] = cos(angle);
    
    return *this;
}

Matrix4 Matrix4::makeRotateY(float angle)
{
    identity();
    
	m[0][0] = cos(angle);
	m[0][2] = -sin(angle);
	m[2][0] = sin(angle);
	m[2][2] = cos(angle);
    
    return *this;
}

Matrix4 Matrix4::makeRotateZ(float angle)
{
    identity();
    
    //Configure this matrix to be a rotation about the Z-Axis by 'angle' radians
	m[0][0] = cos(angle);
	m[0][1] = sin(angle);
	m[1][0] = -sin(angle);
	m[1][1] = cos(angle);
    
    return *this;
}

Matrix4 Matrix4::makeRotateArbitrary(Vector3 a, float angle)
{
    identity();
    
    //Configure this matrix to be a rotation about the 'a' axis by 'angle' radians
	m[0][0] = 1 + (1 - cos(angle))*(a[0] * a[0] - 1);
	m[0][1] = a[2] * sin(angle) + (1 - cos(angle))*a[1] * a[0];
	m[0][2] = -a[1] * sin(angle) + (1 - cos(angle))*a[2] * a[0];
	m[1][0] = -a[2] * sin(angle) + (1 - cos(angle))*a[0] * a[1];
	m[1][1] = 1 + (1 - cos(angle)) * (a[1] * a[1] - 1);
	m[1][2] = a[0] * sin(angle) + (1 - cos(angle))*a[2] * a[1];
	m[2][0] = a[1] * sin(angle) + (1 - cos(angle))*a[0] * a[2];
	m[2][1] = -a[0] * sin(angle) + (1 - cos(angle))*a[1] * a[2];
	m[2][2] = 1 + (1 - cos(angle))*(a[2] * a[2] - 1);
    
    return *this;
}

Matrix4 Matrix4::makeScale(float s)
{
    return makeScale(s, s, s);
}

Matrix4 Matrix4::makeScale(float sx, float sy, float sz)
{
    identity();
    
    //Configure this matrix to be a sclaing by sx, sy, sz
	m[0][0] = sx;
	m[1][1] = sy;
	m[2][2] = sz;

    return *this;
}

Matrix4 Matrix4::makeTranslate(float x, float y, float z)
{
    identity();
    
    //Configure this matrix to be a translation by vector 'a'
	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
    
    return *this;
}

Matrix4 Matrix4::makeTranslate(Vector3 a)
{
    return makeTranslate(a[0], a[1], a[2]);
}

Matrix4 Matrix4::transpose(void)
{
    Matrix4 b;
    for(int x = 0; x < 4; ++x)
    {
        for(int y = 0; y < 4; ++y)
        {
            b.m[y][x] = m[x][y];
        }
    }
    return b;
}

Matrix4 Matrix4::transpose3(void){
	Matrix4 b;
	for (int x = 0; x < 3; x++){
		for (int y = 0; y < 3; y++){
			b.m[y][x] = m[x][y];
		}
	}
	b.m[3][3] = 1;
	return b;
}



//Hint: Try basing this on code by cool people on the internet
//In this class it is okay to use code from the internet
//So long as you fully understand the code and can clearly explain it if asked to
//http://stackoverflow.com/questions/2624422/efficient-4x4-matrix-inverse-affine-transform
Matrix4 Matrix4::inverse(void)
{
    Matrix4 b;
	/*
    //Calculate the inverse of this matrix
	float s0 = m[0][0] * m[1][1] - m[0][1] * m[1][0];
	float s1 = m[0][0] * m[2][1] - m[0][1] * m[2][0];
	//var s2 : Number = i00 * i13 - i10 * i03;
	float s2 = m[0][0] * m[3][1] - m[0][1] * m[3][0];
	//var s3 : Number = i01 * i12 - i11 * i02;
	float s3 = m[1][0] * m[2][1] - m[1][1] * m[2][0];
	//var s4 : Number = i01 * i13 - i11 * i03;
	float s4 = m[1][0] * m[3][1] - m[1][1] * m[3][0];
	//var s5 : Number = i02 * i13 - i12 * i03;
	float s5 = m[2][0] * m[3][1] - m[2][1] * m[3][0];

	//var c5 : Number = i22 * i33 - i32 * i23;
	float c5 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
	//var c4 : Number = i21 * i33 - i31 * i23;
	float c4 = m[1][2] * m[3][3] - m[1][3] * m[3][2];
	//var c3 : Number = i21 * i32 - i31 * i22;
	float c3 = m[1][2] * m[2][3] - m[1][3] * m[2][2];
	//var c2 : Number = i20 * i33 - i30 * i23;
	float c2 = m[0][2] * m[3][3] - m[0][3] * m[3][2];
	//var c1 : Number = i20 * i32 - i30 * i22;
	float c1 = m[0][2] * m[2][3] - m[0][3] * m[2][2];
	//var c0 : Number = i20 * i31 - i30 * i21;
	float c0 = m[0][2] * m[1][3] - m[0][3] * m[1][2];

	// Should check for 0 determinant

	//var invdet : Number = 1 / (s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0);
	float invdet = 1 / (s0*c5 - s1*c4 + s2*c3 + s3*c2 - s4*c1 + s5*c0);

	//m.i00 = (i11 * c5 - i12 * c4 + i13 * c3) * invdet;
	b.m[0][0] = (m[1][1] * c5 - m[2][1] * c4 + m[3][1] * c3)*invdet;
	//m.i01 = (-i01 * c5 + i02 * c4 - i03 * c3) * invdet;
	b.m[1][0] = (-m[1][0] * c5 + m[2][0] * c4 - m[3][0] * c3)*invdet;
	//m.i02 = (i31 * s5 - i32 * s4 + i33 * s3) * invdet;
	b.m[2][0] = (m[3][1] * s5 - m[2][3] * s4 + m[3][3] * s3)*invdet;
	//m.i03 = (-i21 * s5 + i22 * s4 - i23 * s3) * invdet;
	b.m[3][0] = (-m[1][2] * s5 + m[2][2] * s4 - m[3][2] * s3)*invdet;

	//m.i10 = (-i10 * c5 + i12 * c2 - i13 * c1) * invdet;
	b.m[0][1] = (-m[0][1] * c5 + m[2][1] * c2 - m[3][1] * c1)*invdet;
	//m.i11 = (i00 * c5 - i02 * c2 + i03 * c1) * invdet;
	b.m[1][1] = (m[0][0] * c5 - m[2][0] * c2 + m[3][0] * c1)*invdet;
	//m.i12 = (-i30 * s5 + i32 * s2 - i33 * s1) * invdet;
	b.m[2][1] = (-m[0][3] * s5 + m[2][3] * s2 - m[3][3] * s1)*invdet;
	//m.i13 = (i20 * s5 - i22 * s2 + i23 * s1) * invdet;
	b.m[1][1] = (m[0][2] * c5 - m[2][2] * s2 + m[3][2] * s1)*invdet;

	//m.i20 = (i10 * c4 - i11 * c2 + i13 * c0) * invdet;
	//m.i21 = (-i00 * c4 + i01 * c2 - i03 * c0) * invdet;
	//m.i22 = (i30 * s4 - i31 * s2 + i33 * s0) * invdet;
	//m.i23 = (-i20 * s4 + i21 * s2 - i23 * s0) * invdet;

	//m.i30 = (-i10 * c3 + i11 * c1 - i12 * c0) * invdet;
	//m.i31 = (i00 * c3 - i01 * c1 + i02 * c0) * invdet;
	//m.i32 = (-i30 * s3 + i31 * s1 - i32 * s0) * invdet;
	//m.i33 = (i20 * s3 - i21 * s1 + i22 * s0) * invdet;
	*/
	return b;
}

Matrix4 Matrix4::orthoNormalInverse(void)
{
    Matrix4 b;
    
    //Calculate the inverse of this matrix with the assumption that it is ortho-normal
    //This will be useful when implementing cameras!
	Vector4 t =  multiply(Vector4(0, 0, 0, 1));
	b = transpose3();
	b = b * makeTranslate(t.toVector3().negate());
    
    return b;
}

void Matrix4::print(std::string comment)
{
    //Width constants and variables
    static const int pointWidth = 1;
    static const int precisionWidth = 4;
    int integerWidth = 1;
    
    //Determine the necessary width to the left of the decimal point
    float* elementPtr = (float*)m;
    float maxValue = fabsf(*(elementPtr++));
    while(elementPtr++ < ((float*)m+16)) if(fabsf(*elementPtr) > maxValue) maxValue = fabsf(*elementPtr);
    while(maxValue >= 10.0) { ++integerWidth; maxValue /= 10.0; }
    
    //Sum up the widths to determine the cell width needed
    int cellWidth = integerWidth + pointWidth + precisionWidth;
    
    //Set the stream parameters for fixed number of digits after the decimal point
    //and a set number of precision digits
    std::cout << std::fixed;
    std::cout << std::setprecision(precisionWidth);
    
    //Print the comment
    std::cout << comment << std::endl;
    
    //Loop through the matrix elements, format each, and print them to screen
    float cellValue;
    for(int element = 0; element < 4; element++)
    {
        std::cout << std::setw(1) << (element == 0 ? "[" : " ");
        for(int vector = 0; vector < 4; vector++)
        {
            cellValue =  m[vector][element];
            std::cout << std::setw(cellWidth + (cellValue >= 0.0 ? 1 : 0)) << cellValue;
            std::cout << std::setw(0) << (vector < 3 ? " " : "");
        }
        std::cout << std::setw(1) << (element == 3 ? "]" : " ") << std::endl;
    }
}
