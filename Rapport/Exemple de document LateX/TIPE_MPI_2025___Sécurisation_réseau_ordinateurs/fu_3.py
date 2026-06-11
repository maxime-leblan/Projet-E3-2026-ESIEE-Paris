def decomposition_chaine(chaine: str, separateur="|"):
    """ Fonction qui prend en paramètre une chaîne de caractères que l'on peut diviser en sous-chaînes séparées par un séparateur ':' et renvoie une liste contenant ces sous-chaînes """
    liste_sous_chaines = []
    arg_courant = ""
    for i in range(len(chaine)):
        if chaine[i] != separateur:
            arg_courant += chaine[i]
        else:
            liste_sous_chaines.append(arg_courant)
            arg_courant = ""
    if arg_courant != "":
        liste_sous_chaines.append(arg_courant)
    return liste_sous_chaines

def id_client(chaine: str):
    """ Fonction qui prend une chaine de caractères de type '<Client- * > *** ' et renvoie l'entier * en chaine de caractères """
    chiffres = "0123456789"
    id_passe = False
    id = ""
    for char in chaine:
        if char in chiffres:
            id_passe = True
            id += char
        elif id_passe:
            break
    return id