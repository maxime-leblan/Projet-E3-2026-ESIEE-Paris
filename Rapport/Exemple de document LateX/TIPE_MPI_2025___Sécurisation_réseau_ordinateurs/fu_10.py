def affiche_matrice(m: list[list]):
    """ Fonction qui affiche une matrice carrée """
    for i in range(len(m)):
        for j in range(len(m)):
            print(m[i][j], " ", end="")
        print()

def decale_liste(liste: list, d: int):
    """ Fonction qui décale les éléments de la liste de 'd' vers la droite
    et renvoie le résultat"""
    liste_bis = liste.copy()
    liste_finale = []
    d_bis = d

    if d == 0:
        return liste

    while d_bis > 0:
        liste_finale = []
        liste_finale.append(liste_bis.pop())
        for el in liste_bis:
            liste_finale.append(el)
        liste_bis = liste_finale.copy()
        d_bis -= 1

    return liste_finale