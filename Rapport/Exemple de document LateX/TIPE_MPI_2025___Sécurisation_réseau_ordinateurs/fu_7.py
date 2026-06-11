def conversion_matrice(matrice: list[list], fonction, base=10):
    """ Fonction qui applique la fonction de conversion 'fonction' sur chaque case de la matrice carrée """
    res = matrice.copy()
    for i in range(len(matrice)):
        for j in range(len(matrice)):
            if base != 10:
                res[i][j] = fonction(res[i][j], base)
            else:
                res[i][j] = fonction(res[i][j])

    return res