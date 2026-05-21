#pragma once

#include <cmath>
#include <ostream>

using namespace std;

struct V2 
{
	float x, y;

	V2(float _x, float _y) { x = _x; y = _y; }
	V2() {} 

	float norm() const          { return sqrt(x*x + y * y); }
	void  normalize()     { float n = norm();  x /= n;    y /= n;    }
	V2    GetNormalized() const { float n = norm();  return V2(x/n,y/n); }
	float getX() { return x; }
	float getY() { return y; }
};

// comparaison sur des flottants... traitement sp�cial

bool operator == (const V2 & a, const V2 & b);

// red�finition des op�rateurs standards

V2 operator + (const V2 & a, const V2 & b);
V2 operator - (const V2 & a, const V2 & b);
V2 operator * (float      a, const V2 & b);
V2 operator * (const V2 & a, float      b);
V2 operator / (const V2 & a, float      b);
V2 operator - (const V2 & a);  // - unaire

// produit scalaire
float prodScal(const V2 & a, const V2 & b);
float prodVect(const V2 & a, const V2 & b);

// vecteur orienté dans l'arc de cercle ayant pour angle pi (donc il est entre [0, pi] sur le cercle trigo)
extern V2 gCoordonateSystemVector;

/*
Indique si le vecteur passé en paramètre est orienté dans la première moitié du cercle trigo (i.e. entre [0, pi])
*/
bool isInFirstHalfOfTrigoCircle(V2 pVector);

/*
Renvoie l'angle correspondant au vecteur passé en paramètre (l'angle appartient à [0, 2*pi])
*/
float getAngle(V2 pVector);

/*
Renvoie un vecteur dont les coordonnées sont (cos(a), sin(a)) si a est l'angle passé en paramètre
pAngle - angle passé en paramètre (en radian)
*/
V2 getVectorFromAngle(float pAngle);

// affichage 
std::ostream & operator << (std::ostream & os, V2 & t);

