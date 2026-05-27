#include <iostream>
#include "V3.h"

float V3::getCoordonateNumber(int pNumber)
{
	if (pNumber == 1)
	{
		return x;
	}
	else if (pNumber == 2)
	{
		return y;
	}
	else
	{
		return z;
	}
}

// comparaison sur des flottants... traitement sp�cial

bool operator == (const V3 & a, const V3 & b)
{
	V3 t = a - b;
	float epsilon = 0.001f;
	return t.norm() < epsilon;
}

// red�finition des op�rateurs standards

V3 operator + (const V3 & a, const V3 & b) {  return V3(a.x + b.x, a.y + b.y, a.z + b.z); }
V3 operator - (const V3 & a, const V3 & b) {  return V3(a.x - b.x, a.y - b.y, a.z - b.z); }
V3 operator * (float      a, const V3 & b) {  return V3(a   * b.x, a   * b.y, a * b.z); }
V3 operator * (const V3 & a, float      b) {  return V3(a.x * b  , a.y * b, a.z * b);   }
V3 operator / (const V3 & a, float      b) {  return V3(a.x / b  , a.y / b, a.z / b);   }
V3 operator - (const V3 & a)               {  return V3( -a.x, -a.y, -a.z); }


// produit scalaire / vectoriel
 
float prodScal(const V3 & a, const V3 & b)  { return a.x * b.x + a.y * b.y + a.z * b.z; }
V3 prodVect(const V3 & a, const V3 & b)  { return V3(a.y * b.z - b.y * a.z, a.z * b.x - b.z * a.x, a.x * b.y - b.x * a.y); }

// V3 gCoordonateSystemVector = V3(0, 1);

/*
bool isInFirstHalfOfTrigoCircle(V3 pVector)
{
	return prodScal(gCoordonateSystemVector, pVector) >= 0;
}
*/
/*
float getAngle(V3 pVector)
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
*/


// V3 getVectorFromAngle(float pAngle) { return V3(cos(pAngle), sin(pAngle)); }

// affichage 

std::ostream & operator << (std::ostream & os, V3 & t)
{
	os << "(" << t.x << "," << t.y << "," << t.z << ")";
	return os;
}