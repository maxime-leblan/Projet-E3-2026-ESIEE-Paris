#include "GridItem.h"

GridItem::GridItem(string pName, V3 pPosition)
{
    aName = pName;
    aPosition = pPosition;
}

V3 GridItem::getPosition() const
{
    return aPosition;
}

void GridItem::setPosition(V3 pNewPosition)
{
    aPosition = pNewPosition;
}