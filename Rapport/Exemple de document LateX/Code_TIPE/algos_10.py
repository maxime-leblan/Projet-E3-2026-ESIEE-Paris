def bytesub(matrice: list[list]):
    """ Fonction bytesub qui prend en entrée une matrice codée en hexadécimal et renvoie le résultat
    de la fonction aux_bytesub de AES appliquée à celle-ci """

    return [[aux_bytesub(matrice[i][j]) for j in range(len(matrice))] for i in range(len(matrice))]

def shiftrows(matrice: list[list]):
    """ Fonction qui applique la fonction ShiftRows de AES sur la matrice """
    n = len(matrice)
    matrice_finale = []
    for d in range(n):
        matrice_finale.append(fu.decale_liste(matrice[d], d))

    return matrice_finale