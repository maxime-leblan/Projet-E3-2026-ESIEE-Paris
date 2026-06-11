def creation_cles(lst_nb_prem: list):
    """ Fonction prenant en paramètre une liste de nombres premiers et qui crée puis renvoie un couple de la forme
     (cle_publique, cle_privee) en suivant la méthode de création des clés de l'algorithme de chiffrement RSA\n
     - cle_publique : tuple int \n
     - cle_privee : int """
    e = 2
    # On choisit aléatoirement les valeurs de p et q distinctes
    p = lst_nb_prem.pop(rd.randint(0, len(lst_nb_prem) - 1))
    q = rd.choice(lst_nb_prem)
    # On choisit e et phi_n
    n = p * q
    phi_n = (p - 1) * (q - 1)

    for i in range(2, phi_n): # on cherche le premier e qui convient en partant de e = 2
        if fu.pgcd(i, phi_n) == 1:
            e = i
            break
    d = inverse_mod(e, phi_n)
    
    return ((n, e), d) # on renvoie (clé publique, clé privée)