def chiffrer_chaine_RSA(chaine:str, cle_pb:tuple, taille_facteur=-1):
    """ Fonction qui prend en paramètre une chaine de caractères 'chaine' et un tuple 'cle_pb' et renvoie
    une liste d'entiers correspondants aux facteurs de taille 'taille_facteur', chiffrés avec la fonction 'chiffrer_RSA(...)',
    dont la concaténation forme la chaine 'chaine' avant son chiffrement """
    if taille_facteur == (-1):
        taille_facteur = len(chaine)
    chaine_chiffree = []
    chaine_fractionnee = []
    chaine_courante = ""

    for i in range(len(chaine)): # On décompose 'chaine' en une liste de facteurs de taille 'taille_facteur'
        chaine_courante += chaine[i]
        if len(chaine_courante) % taille_facteur == 0:
            chaine_fractionnee.append(chaine_courante)
            chaine_courante = ""

    if chaine_courante != "":
        for i in range(len(chaine_courante), taille_facteur): # on ajoute des espaces pour que le dernier facteur soit de taille taille_facteur
            chaine_courante += " "
        chaine_fractionnee.append(chaine_courante) # on ajoute le dernier facteur de taille < 'taille_facteur' (sans compter les espaces)

    for facteur in chaine_fractionnee: # On chiffre tous les facteurs avec la fonction 'chiffrer_RSA(...)'
        int_facteur_courant = string_to_int(facteur, base=95)
        chaine_chiffree.append(chiffrer_RSA_v2(int_facteur_courant, cle_pb))
    return chaine_chiffree