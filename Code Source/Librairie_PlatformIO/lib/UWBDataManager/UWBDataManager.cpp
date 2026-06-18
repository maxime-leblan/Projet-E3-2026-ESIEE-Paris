#include "UWBDataManager.hpp"

std::vector<int> getDataFromString(const std::string& texte, const std::string& patternRegex) {
    std::vector<int> entiersExtraits;
    
    try {
        // Compilation de la regex
        std::regex regex(patternRegex);
        
        // Création des itérateurs pour parcourir les correspondances
        auto debut_match = std::sregex_iterator(texte.begin(), texte.end(), regex);
        auto fin_match = std::sregex_iterator();

        // Boucle sur toutes les correspondances trouvées
        for (std::sregex_iterator i = debut_match; i != fin_match; ++i) {
            std::smatch match = *i;
            try {
                // Conversion de la chaîne correspondante en entier (int)
                entiersExtraits.push_back(std::stoi(match.str()));
            } 
            catch (const std::invalid_argument& e) {
                std::cerr << "Avertissement : '" << match.str() << "' n'est pas un entier valide." << std::endl;
            } 
            catch (const std::out_of_range& e) {
                std::cerr << "Avertissement : '" << match.str() << "' est trop grand pour un type int." << std::endl;
            }
        }
    } 
    catch (const std::regex_error& e) {
        std::cerr << "Erreur de syntaxe dans la regex : " << e.what() << std::endl;
    }

    return entiersExtraits;
}

std::vector<float> getFloatDataFromString(const std::string& texte, const std::string& patternRegex) {
    std::vector<float> entiersExtraits;
    
    try {
        // Compilation de la regex
        std::regex regex(patternRegex);
        
        // Création des itérateurs pour parcourir les correspondances
        auto debut_match = std::sregex_iterator(texte.begin(), texte.end(), regex);
        auto fin_match = std::sregex_iterator();

        // Boucle sur toutes les correspondances trouvées
        for (std::sregex_iterator i = debut_match; i != fin_match; ++i) {
            std::smatch match = *i;
            try {
                // Conversion de la chaîne correspondante en flottant (float)
                entiersExtraits.push_back(std::stof(match.str()));
            } 
            catch (const std::invalid_argument& e) {
                std::cerr << "Avertissement : '" << match.str() << "' n'est pas un flottant valide." << std::endl;
            } 
            catch (const std::out_of_range& e) {
                std::cerr << "Avertissement : '" << match.str() << "' est trop grand pour un type float." << std::endl;
            }
        }
    } 
    catch (const std::regex_error& e) {
        std::cerr << "Erreur de syntaxe dans la regex : " << e.what() << std::endl;
    }

    return entiersExtraits;
}

float getDistanceFromUWBMessage(std::vector<float> pUWBMessageData)
{
    return pUWBMessageData[UWB_MESSAGE_DISTANCE_INDEX];
}

int getOrderTypeFromUWBMessage(std::vector<float> pUWBMessageData)
{
    return (int)pUWBMessageData[UWB_MESSAGE_ORDER_TYPE_INDEX];
}

int getReceiverIdFromUWBMessage(std::vector<float> pUWBMessageData)
{
    return (int)pUWBMessageData[UWB_MESSAGE_RECEIVER_ID_INDEX];
}

int getSenderIdFromUWBMessage(std::vector<float> pUWBMessageData)
{
    return (int)pUWBMessageData[UWB_MESSAGE_SENDER_ID_INDEX];
}

int getDistanceFromAnchor(std::vector<int> pTagData, int pAnchorId)
{
    return pTagData[FIRST_TAG_DISTANCE_INDEX + pAnchorId];
}

int getTagIdFromTagData(std::vector<int> pTagData)
{
    return pTagData[TAG_ID_INDEX];
}