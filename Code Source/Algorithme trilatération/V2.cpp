#include <iostream>
#include "V2.h"

// comparaison sur des flottants... traitement sp�cial

bool operator == (const V2 & a, const V2 & b)
{
	V2 t = a - b;
	float epsilon = 0.001f;
	return t.norm() < epsilon;
}

// red�finition des op�rateurs standards

V2 operator + (const V2 & a, const V2 & b) {  return V2(a.x + b.x, a.y + b.y); }
V2 operator - (const V2 & a, const V2 & b) {  return V2(a.x - b.x, a.y - b.y); }
V2 operator * (float      a, const V2 & b) {  return V2(a   * b.x, a   * b.y); }
V2 operator * (const V2 & a, float      b) {  return V2(a.x * b  , a.y * b);   }
V2 operator / (const V2 & a, float      b) {  return V2(a.x / b  , a.y / b);   }
V2 operator - (const V2 & a)               {  return V2( -a.x, -a.y ); }


// produit scalaire / vectoriel
 
float prodScal(const V2 & a, const V2 & b)  { return a.x * b.x + a.y * b.y; }
float prodVect(const V2 & a, const V2 & b)  { return a.x * b.y - a.y * b.x; }

V2 gCoordonateSystemVector = V2(0, 1);

bool isInFirstHalfOfTrigoCircle(V2 pVector)
{
	return prodScal(gCoordonateSystemVector, pVector) >= 0;
}

float getAngle(V2 pVector)
{
	float vFirstHalfAngle = acos(pVector.getX());
	if (isInFirstHalfOfTrigoCircle(pVector))
	{
		return vFirstHalfAngle;
	}
	else
	{
		return vFirstHalfAngle + 2*(M_PI - vFirstHalfAngle);
	}
}

V2 getVectorFromAngle(float pAngle) { return V2(cos(pAngle), sin(pAngle)); }

// affichage 

std::ostream & operator << (std::ostream & os, V2 & t)
{
	os << "(" << t.x << "," << t.y << ")";
	return os;
}