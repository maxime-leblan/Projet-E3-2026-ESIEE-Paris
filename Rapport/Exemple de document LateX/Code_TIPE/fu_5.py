def concat_chaine(chaine:str, n:int):
    """ Fonction qui renvoie la concaténation de 'chaine' (n-1) fois """
    
    chaine_finale = ""
    for _ in range(n):
        chaine_finale += chaine
        
    return chaine_finale

def encode_ascii(chaine:str):
    """ Fonction qui transforme une chaine str en bstr (ie en code ASCII)\n
    NB : elle fait la même chose que la fonction native bytes(chaine, encoding="ascii") """

    return bytes(chaine, encoding="utf-8")