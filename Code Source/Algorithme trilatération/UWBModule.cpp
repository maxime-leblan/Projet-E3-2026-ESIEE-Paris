#include "UWBModule.h"

UWBModule::UWBModule(string pName, V3 pPosition)
{
    aName = pName;
    aPosition = pPosition;
}

string UWBModule::getName() const
{
    return aName;
}

V3 UWBModule::getPosition() const
{
    return aPosition;
}

void UWBModule::setPosition(V3 pNewPosition)
{
    aPosition = pNewPosition;
}