#include "UWBModule.h"

UWBModule::UWBModule() {}

UWBModule::UWBModule(int pId, V3 pPosition)
{
    aId = pId;
    aPosition = pPosition;
}

int UWBModule::getId() const
{
    return aId;
}

V3 UWBModule::getPosition() const
{
    return aPosition;
}

void UWBModule::setPosition(V3 pNewPosition)
{
    aPosition = pNewPosition;
}