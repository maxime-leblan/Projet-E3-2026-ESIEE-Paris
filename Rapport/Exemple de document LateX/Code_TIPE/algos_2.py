def int_to_string(entier: int, taille_mot: int, b=27):
    """ Fonction qui convertit un entier en chaîne de caractères et renvoie cette chaîne """

    quotient = 0
    diviseur = 0
    dividende = entier
    mot = ""

    for i in range(taille_mot - 1, 0, -1):
        diviseur = b**i
        quotient = dividende // diviseur
        mot += liste_int_a_char[quotient]
        dividende -= quotient * diviseur
    mot += liste_int_a_char[dividende%b]

    return mot

# Création de l'exception "InvNotFound" utile dans la suite
class InvNotFound(Exception):
    pass