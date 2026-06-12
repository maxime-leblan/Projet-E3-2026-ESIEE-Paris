#pragma once

#include <cmath>
#include <ostream>

#ifdef ARDUINO
    #include <ArduinoEigenDense.h>
#else
    #include <Eigen/Core> // taper dans le terminal "dpkg -L libeigen3-dev" pour trouver l'emplacement de la librairie
    #include <Eigen/Dense>
#endif

#define NUMBER_OF_COORDONATES 3
#define AXIS_X 1
#define AXIS_Y 2
#define AXIS_Z 3

using namespace std;
// using namespace Eigen;

struct V3 
{
	float x, y, z;

	V3(float _x, float _y, float _z) { x = _x; y = _y; z = _z;}
	V3() {} 

	float norm() const          { return sqrt(x*x + y*y + z*z); }
	float norm2D() const { return sqrt(x*x + y*y); }
	void  normalize()     { float n = norm();  x /= n;    y /= n;    z /= n;}
	V3    getNormalized() const { float n = norm();  return V3(x/n, y/n, z/n); }
	float getX() const { return x; }
	float getY() const { return y; }
	float getZ() const { return z; }
	string toString() const {return "(" + to_string(x) + ", " + to_string(y) + ", " + to_string(z) + ")"; }

	/*
	Renvoie la coordonnée numéro pNumber
	pNumber - indice de la coordonnée que l'on veut récupérer (>= 1)
	*/
	float getCoordonateNumber(int pNumber) const;
};

// comparaison sur des flottants... traitement sp�cial

bool operator == (const V3 & a, const V3 & b);

// red�finition des op�rateurs standards

V3& operator += (V3& a, const V3& b);
V3 operator + (const V3 & a, const V3 & b);
string operator + (string a, const V3 & b);
V3 operator - (const V3 & a, const V3 & b);
V3 operator * (float      a, const V3 & b);
V3 operator * (const V3 & a, float      b);
V3 operator * (const V3 & a, const Eigen::Matrix<float, 3, 3> & b);
V3 operator * (const Eigen::Matrix<float, 3, 3> & a, const V3 & b);
V3 operator / (const V3 & a, float      b);
V3 operator - (const V3 & a);  // - unaire

// produit scalaire
float prodScal(const V3 & a, const V3 & b);
float prodScal2D(const V3 & a, const V3 & b);
V3 prodVect(const V3 & a, const V3 & b);

// vecteur orienté dans l'arc de cercle ayant pour angle pi (donc il est entre [0, pi] sur le cercle trigo)
// extern V3 gCoordonateSystemVector;

/*
Indique si le vecteur passé en paramètre est orienté dans la première moitié du cercle trigo (i.e. entre [0, pi])
*/
// bool isInFirstHalfOfTrigoCircle(V3 pVector);

/*
Renvoie l'angle correspondant au vecteur passé en paramètre (l'angle appartient à [0, 2*pi])
*/
// float getAngle(V3 pVector);

/*
Renvoie un vecteur dont les coordonnées sont (cos(a), sin(a)) si a est l'angle passé en paramètre
pAngle - angle passé en paramètre (en radian)
*/
// V3 getVectorFromAngle(float pAngle);

// affichage 
std::ostream & operator << (std::ostream & os, V3 & t);

