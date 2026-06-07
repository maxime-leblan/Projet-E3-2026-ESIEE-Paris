def chiffrer_RSA_naif(m: int, cle_pb: tuple):
    """ Fonction qui prend en paramètre un entier m qui correspond au message à chiffrer et un tuple cle_pb qui
    correspond à la clé de chiffrement. Elle renvoie un entier correspondant au message chiffré """

    assert (m < cle_pb[0])
    # print("début calcul chiffrement et cle_pb : ", cle_pb)
    reste = 1
    for _ in range(cle_pb[1]):
        reste *= m
        reste = reste % cle_pb[0]
    return reste

def chiffrer_RSA_v2(m: int, cle_pb: tuple):
    """ Fonction qui prend en paramètre un entier m qui correspond au message à chiffrer et un tuple cle_pb qui
    correspond à la clé de chiffrement. Elle renvoie un entier correspondant au message chiffré """

    assert (m < cle_pb[0])
    return modulo_exp(m, cle_pb[1], cle_pb[0])