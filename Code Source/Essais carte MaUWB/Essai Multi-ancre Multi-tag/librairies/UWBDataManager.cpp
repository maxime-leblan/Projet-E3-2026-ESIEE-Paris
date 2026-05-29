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

/*
int getDistanceFromAnchor(string pTagData, int pAnchorId)
{
    char vStartFlag = '(';
    char vEndFlag = ')';
    string vDistance = "-1";
    int i = 0;
    int vCurrentId = 0;
    bool vInTagData = false;

    while (i < BUFFER)
    {
        if (pTagData[i] == vEndFlag)
        {
            return stoi(vDistance);
        }
        else if (vInTagData)
        {
            if (pTagData[i] == ',')
            {
                if (vCurrentId == pAnchorId)
                {
                    return stoi(vDistance);
                }
                else
                {
                    vCurrentId++;
                    vDistance = "";
                }
            }
            else
            {
                vDistance += pTagData[i];
            }
        }
        else if (pTagData[i] == vStartFlag)
        {
            vInTagData = true;
        }
        i++;
    }
}
*/