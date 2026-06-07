def dechiffrer_AES(message_chiffre, cle:bytes, vecteur:bytes):
    """ Fonction qui prend en paramètre un message, une clé et un vecteur et renvoie une chaine de
        caractère correspondant au déchiffrement de 'message_chiffre' avec l'algorithme AES en utilisant la clé 'cle'
        et le vecteur d'initialisation 'vecteur'\n
        - message_chiffre : chaine de caractères\n
        - cle : chaine de caractères sur 32 octets\n
        - vecteur : chaine de caractères sur 16 octets"""

    decryptor = Cipher(algorithms.AES(cle), modes.CBC(vecteur), backend=default_backend()).decryptor()
    decrypted_data = decryptor.update(message_chiffre) + decryptor.finalize()

    unpadder = padding.PKCS7(128).unpadder()
    unpadded_data = unpadder.update(decrypted_data)
    unpadded_data += unpadder.finalize()

    return unpadded_data.decode('utf-8')