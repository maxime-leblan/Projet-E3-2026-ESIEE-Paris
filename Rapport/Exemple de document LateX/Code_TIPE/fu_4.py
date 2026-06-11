def importer_fichier(nom_fichier:str, type="int"):
    """ Importe un fichier texte et ajoute chaque ligne du fichier dans une liste Python que la fonction renvoie """
    liste_nb_premiers = []
    with open(nom_fichier, "r") as fichier:
        contenu = fichier.readlines()
    for ligne in contenu:
        ligne_actuelle = ligne.rstrip("\n")
        if type == "int":
            liste_nb_premiers.append(int(ligne_actuelle))
        else:
            liste_nb_premiers.append(ligne_actuelle)
    return liste_nb_premiers

def generateur_alea_chaine(taille_chaine:int):
    """ Fonction qui génère aléatoirement une chaine de caractères de taille 'taille_chaine', composée des lettres de l'alphabet en minuscule, et la renvoie """
    liste_int_a_char = "abcdefghijklmnopqrstuvwxyz"
    nb_char = len(liste_int_a_char)
    chaine_alea = ""
    for _ in range(taille_chaine):
        i = rd.randint(0, nb_char - 1)
        chaine_alea += liste_int_a_char[i]
    return chaine_alea