def inverse_mod(b: int, n: int):
    """ Fonction qui prend en paramètre deux entiers b et n et renvoie l'inverse de b modulo n (ie b^(-1))
    ou bien renvoie l'erreur 'InvNotFound' si b^(-1) n'existe pas """
    n_0 = n
    b_0 = b
    t_0 = 0
    t = 1
    q = int(n_0/b_0)
    r = n_0 - q * b_0
    while r > 0:
        temp = t_0 - q * t
        if temp >= 0:
            temp = temp % n
        else:
            temp = n - ((-temp) % n)
        t_0 = t
        t = temp
        n_0 = b_0
        b_0 = r
        q = int(n_0/b_0)
        r = n_0 - q * b_0
    if b_0 != 1:
        raise InvNotFound()
    else:
        return t