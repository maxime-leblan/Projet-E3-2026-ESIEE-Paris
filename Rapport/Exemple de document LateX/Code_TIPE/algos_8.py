def dechiffrer_chaine_RSA(liste:list, cle_pv: int, n: int, taille_entiers=1):
    """ Fonction qui prend en paramètre une liste d'entiers 'liste', un entier 'cle_pv' et un entier n et
    renvoie la concaténation des chaines de caractères déchiffrées avec 'dechiffrer_RSA(...)' à partir des entiers
    contenu dans 'liste' représentant des chaines de caractères de taille 'taille_entiers' """

    chaine_dechiffree = ""

    for i in range(len(liste)): # On déchiffre tous les termes de liste sauf le dernier
        chaine_courante = int_to_string(dechiffrer_RSA_v2(liste[i], cle_pv, n), taille_entiers, b=95)
        chaine_dechiffree += chaine_courante

    return chaine_dechiffree