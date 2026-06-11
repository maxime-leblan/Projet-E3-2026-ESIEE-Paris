def matrice_gf(chaine: str, taille=16):
    """ Renvoie la matrice GF sous forme de matrice de type list[list] """
    matrice_gf = decomposition_chaine(chaine, separateur=" ")
    matrice_gf_bis = []
    matrice_gf_finale = []
    ligne_gf = []

    for i in range(len(matrice_gf)):
        if "\n" in matrice_gf[i]:
            matrice_gf_bis.append(matrice_gf[i][:(-1)])
        elif matrice_gf[i] != "":
            matrice_gf_bis.append(matrice_gf[i])

    for i in range(taille):
        for j in range(taille):
            ligne_gf.append(matrice_gf_bis[i*taille + j])
            if j == taille - 1:
                matrice_gf_finale.append(ligne_gf)
                ligne_gf = []

    return matrice_gf_finale