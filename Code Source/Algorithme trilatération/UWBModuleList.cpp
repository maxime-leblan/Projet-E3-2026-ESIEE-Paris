#include "UWBModuleList.h"

UWBModuleList::UWBModuleList() {}

int UWBModuleList::size()
{
    return aUWBModuleList.size();
}

UWBModule UWBModuleList::getModule(int pId)
{
    return aUWBModuleList[pId];
}

void UWBModuleList::addModule(int pId, UWBModule pModule)
{
    aUWBModuleList[pId] = pModule;
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