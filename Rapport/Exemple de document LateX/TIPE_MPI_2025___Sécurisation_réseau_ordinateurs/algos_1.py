def aux_modulo(x, n, m, acc_reste):
    """ Calcul de x^n modulo m avec la méthode d'exponentiation rapide"""
    if n == 1:
        return acc_reste % m
    elif n % 2 == 0:
        return aux_modulo(x*x % m, int(n/2), m, acc_reste*x % m)
    else:
        return aux_modulo(x*x % m, int((n-1)/2), m, acc_reste*x*x % m)

def modulo_exp(x:int, n:int, m:int):
    """ Fonction qui calcule x^n modulo m """
    if n == 0:
        return 1 % m
    else:
        return aux_modulo(x, n, m, x)

def string_to_int(chaine: str, base=27):
    """ Fonction qui convertit une chaîne de caractères en entier et renvoie cet entier """
    n = len(chaine)
    entier = 0
    
    for k in range(1, n+1):
        entier += dico_char_a_int[chaine[k-1]] * base**(n - k)
    return entier
