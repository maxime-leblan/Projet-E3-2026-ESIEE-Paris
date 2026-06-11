def bin_vers_dec(x:str):
    """ Fonction qui convertit un élément de type int binaire en élément de type int décimal"""
    x_str = str(x)
    x_liste = [int(c) for c in x_str]
    x_liste.reverse()
    x_int = 0
    k = 0
    for c in x_liste:
        x_int += 2**k * c
        k += 1

    return x_int

def remplir_zeros(x, n: int):
    """ Rajoute des zéros jusqu'à ce que 'x' soit un nombre binaire codé sur n bits """
    x_str = str(x)
    while len(x_str) < n:
        x_str = "0" + x_str

    return x_str