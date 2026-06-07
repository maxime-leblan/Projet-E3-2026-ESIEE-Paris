def aux_bytesub(a: str):
    """ Fonction auxiliaire de bytesub qui prend en entrée un élément codé en hexadécimal et renvoie le résultat
    de la fonction ByteSub appliquée à celui-ci """

    # on construit b_prime = GF(a)
    m_gf_i = m_gf_hex_a_dec[a[0]]
    m_gf_j = m_gf_hex_a_dec[a[1]]
    b_prime = MATRICE_GF[m_gf_i][m_gf_j]
    # on construit b à partir de b_prime
    b_prime = int(b_prime, 16)
    b_prime = str(bin(b_prime))
    b_prime = b_prime[2:]
    b_prime = fu.remplir_zeros(b_prime, 8)
    b_prime = [[int(b_prime[i])] for i in range(len(b_prime))]
    b = fu.produit_matriciel(mat_f1, b_prime, (len(mat_f1), len(mat_f1)), (len(b_prime), 1))
    b = fu.somme_matricielle(b, mat_f2, len(mat_f2), 1)
    for i in range(len(b)):
        b[i][0] = b[i][0] % 2
    b_final = ""
    for chiffre in b:
        b_final += str(chiffre[0])
    b_final = fu.remplir_zeros(b_final, 8)
    b_final = fu.bin_vers_dec(b_final)
    b_final = str(hex(b_final))

    return b_final[2:]