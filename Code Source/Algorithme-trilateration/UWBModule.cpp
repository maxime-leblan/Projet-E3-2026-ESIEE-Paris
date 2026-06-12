#include "UWBModule.h"

UWBModule::UWBModule() {}

UWBModule::UWBModule(int pId)
{
    aId = pId;
    aPosition = V3(0, 0, 0);
}

UWBModule::UWBModule(int pId, V3 pPosition)
{
    aId = pId;
    aPosition = pPosition;
}

int UWBModule::getId() const
{
    return aId;
}

string UWBModule::toString()
{
    return "(" + to_string(aId) + ", " + aPosition + ")";
}

V3 UWBModule::getPosition() const
{
    return aPosition;
}

void UWBModule::setPosition(V3 pNewPosition)
{
    aPosition = pNewPosition;
}