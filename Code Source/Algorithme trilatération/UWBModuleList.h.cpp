#include "UWBModuleList.h"

UWBModuleList::UWBModuleList() {}

UWBModule UWBModuleList::getModule(string pModuleName)
{
    return aUWBModuleList[pModuleName];
}

void UWBModuleList::addModule(string pModuleName, UWBModule pModule)
{
    aUWBModuleList[pModuleName] = pModule;
}

vector<string>UWBModuleList::findAll(bool (&pFunction)(UWBModule))
{
    vector<string> vResTab;

    for (auto it = aUWBModuleList.begin(); it != aUWBModuleList.end(); it++)
    {
        if (pFunction(it->second))
        {
            vResTab.push_back(it->first);
        }
    }

    return vResTab;
}