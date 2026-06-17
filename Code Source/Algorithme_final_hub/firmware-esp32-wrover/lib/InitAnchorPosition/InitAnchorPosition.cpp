#include "InitAnchorPosition.hpp"

void toggleAnchorsMode(std::vector<int> pAnchorsId)
{
    for (int i = 0; i < pAnchorsId.size(); i++)
    {
        sendCanOrderFromHubTo(pAnchorsId[i], HUB_ORDER_TOGGLE_MODULE_MODE);
    }
}