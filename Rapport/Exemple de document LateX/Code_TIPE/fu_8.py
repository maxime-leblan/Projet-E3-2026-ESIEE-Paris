def produit_matriciel(m1: list[list[int]], m2: list[list[int]], n1=(-1, -1), n2=(-1, -1)):
    """ Fonction qui renvoie le produit matriciel des matrices m1 et m2 """
    assert(n1[1] == n2[0])
    if n1 == (-1,-1):
        n = len(m1)
        p = n
        q = n
    else:
        n, p = n1[0], n1[1]
        q = n2[1]
    res = [[0 for _ in range(q)] for _ in range(n)]
    for i in range(n):
        for j in range(q):
            somme = 0
            for k in range(p):
                somme += m1[i][k] * m2[k][j]
            res[i][j] = somme
    return res

def somme_matricielle(m1: list[list[int]], m2: list[list[int]], n: int, p: int):
    """ Fonction qui renvoie la somme des matrices m1 et m2 """
    res = [[0 for _ in range(p)] for _ in range(n)]
    for i in range(n):
        for j in range(p):
            res[i][j] = m1[i][j] + m2[i][j]
    return res