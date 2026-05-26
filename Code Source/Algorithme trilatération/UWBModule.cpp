#include "UWBModule.h"

UWBModule::UWBModule(string pName, V3 pPosition)
{
    aName = pName;
    aPosition = pPosition;
}

V3 UWBModule::getPosition() const
{
    return aPosition;
}

void UWBModule::setPosition(V3 pNewPosition)
{
    aPosition = pNewPosition;
}