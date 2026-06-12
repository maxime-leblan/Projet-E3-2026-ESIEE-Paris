#include "UWBModuleList.h"

UWBModuleList::UWBModuleList() {}

int UWBModuleList::size()
{
    return aUWBModuleList.size();
}

std::map<int, V3> UWBModuleList::giveModulePositionList() const
{
    std::map<int, V3> vModulePositionList;

    for (auto it = aUWBModuleList.begin(); it != aUWBModuleList.end(); it++)
    {
        vModulePositionList[it->first] = it->second.getPosition();
    }

    return vModulePositionList;    
}

string UWBModuleList::toString()
{
    string vString = "[";

    for (auto it = aUWBModuleList.begin(); it != aUWBModuleList.end(); it++)
    {
        vString += (it->second).toString() + ", ";
    }

    if (vString.size() > 1)
    {
        vString.pop_back();
        vString.pop_back();
    }

    return vString + "]";
}

UWBModule UWBModuleList::getModule(int pId)
{
    return aUWBModuleList[pId];
}

void UWBModuleList::addModule(int pId, UWBModule pModule)
{
    aUWBModuleList[pId] = pModule;
}

void UWBModuleList::setModulePosition(int pModuleId, V3 pNewPosition)
{
    aUWBModuleList[pModuleId].setPosition(pNewPosition);
}

vector<int>UWBModuleList::findAll(bool (&pFunction)(UWBModule))
{
    vector<int> vResTab;

    for (auto it = aUWBModuleList.begin(); it != aUWBModuleList.end(); it++)
    {
        if (pFunction(it->second))
        {
            vResTab.push_back(it->first);
        }
    }

    return vResTab;
}

vector<int> UWBModuleList::giveModuleIdList()
{
    vector<int> vModuleIdList;

    for (auto it = aUWBModuleList.begin(); it != aUWBModuleList.end(); it++)
    {
        vModuleIdList.push_back(it->first);
    }

    return vModuleIdList;
}