# Algorithme AES implémenté avec une bibliothèque
def chiffrer_AES(message:str, cle:bytes, vecteur:bytes):
    """ Fonction qui prend en paramètre un message, une clé et un vecteur et renvoie une chaine de
    caractère correspondant au chiffrement de 'message' avec l'algorithme AES en utilisant la clé 'cle'
    et le vecteur d'initialisation 'vecteur'\n
    - message : chaine de caractères\n
    - cle : chaine de caractères sur 32 octets\n
    - vecteur : chaine de caractères sur 16 octets"""
    
    padder = padding.PKCS7(128).padder()
    padded_data = padder.update(message.encode('utf-8'))
    padded_data += padder.finalize()
    
    cipher = Cipher(algorithms.AES(cle), modes.CBC(vecteur), backend=default_backend())
    encryptor = cipher.encryptor()
    message_chiffre = encryptor.update(padded_data) + encryptor.finalize()
    
    return message_chiffre