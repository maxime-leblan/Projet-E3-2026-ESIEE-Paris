def dechiffrer_RSA_naif(x: int, cle_pv: int, n: int):
    """ Fonction prenant en paramètre un entier x correspondant au message à déchiffrer, cle_pv un entier
    correspondant à la clé de déchiffrement et n issue de la cle de chiffrement du message. Elle renvoie un entier
    correspondant au message déchiffré """

    reste = 1
    for _ in range(cle_pv):
        reste *= x
        reste = reste % n
        
    return reste

def dechiffrer_RSA_v2(x: int, cle_pv: int, n: int):
    """ Fonction prenant en paramètre un entier x correspondant au message à déchiffrer, cle_pv un entier
    correspondant à la clé de déchiffrement et n issue de la cle de chiffrement du message. Elle renvoie un entier
    correspondant au message déchiffré """

    return modulo_exp(x, cle_pv, n)