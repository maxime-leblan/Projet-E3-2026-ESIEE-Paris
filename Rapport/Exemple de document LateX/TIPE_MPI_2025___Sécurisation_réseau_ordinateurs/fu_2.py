def compression_arg_str(liste_args: list, separateur="/"):
    """ Fonction qui prend en paramètre une liste d'objets pouvant être convertis en chaine de caractères, un séparateur '/' et renvoie une chaine de caractères contenant n paramètres et qui est de la forme 'arg[1]/arg[2]/.../arg[n]' """
    arguments = ""
    for i in range(len(liste_args)):
        if i != len(liste_args) - 1:
            arguments += str(liste_args[i]) + separateur
        else:
            arguments += str(liste_args[i])
    return arguments

def extraction_arg_str(chaine_args: str, separateur="/"):
    """ Fonction qui prend en paramètre une chaine de caractères contenant n arguments et de la forme 'arg[1]/arg[2]/.../arg[n]', un séparateur '/' et renvoie une liste contenant les n arguments de type int"""
    chiffres = "0123456789"
    liste_args = []
    arg_courant = ""
    for i in range(len(chaine_args)):
        if chaine_args[i] != separateur and chaine_args[i] in chiffres:
            arg_courant += chaine_args[i]
        else:
            liste_args.append(int(arg_courant))
            arg_courant = ""
    if arg_courant != "":
        liste_args.append(int(arg_courant))
    return liste_args