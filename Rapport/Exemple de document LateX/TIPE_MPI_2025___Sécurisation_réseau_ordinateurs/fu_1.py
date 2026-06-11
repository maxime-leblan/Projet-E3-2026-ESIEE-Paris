def copie_dict(dico:dict):
    """ Fonction qui renvoie une copie indépendante du dictionnaire 'dico' """
    dico_copie = {}
    for cle, valeur in dico.items():
        dico_copie[cle] = valeur

    return dico_copie

def pgcd(a: int, b: int):
    """ Fonction qui prend en paramètre 2 entiers naturel a et b et renvoie leur PGCD """

    dividende = max(a, b)
    diviseur = min(a, b)
    temp = 0
    while (dividende % diviseur) != 0 :
        temp = dividende % diviseur
        dividende = diviseur
        diviseur = temp

    return diviseur