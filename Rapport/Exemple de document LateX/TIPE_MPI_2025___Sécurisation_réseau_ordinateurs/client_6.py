def connection(self):
        """ Fonction permettant de connecter le client à un serveur """
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((self.host, self.port))
        envoi = threading.Thread(target=self.envoyer, args=[self.socket])
        recep = threading.Thread(target=self.reception, args=[self.socket])
        envoi.start()
        recep.start()

class Client_AES_RSA(Client_RSA):
    """ Classe fille de Client_RSA permettant de simuler un client pouvant se connecter à un serveur informatique simplifié
    à l'aide du module socket, qui base le chiffrement sur une combinaison de l'algorithme RSA et AES """
    def __init__(self, id: int, liste_nb_premiers):
        Client_RSA.__init__(self, id, liste_nb_premiers)
        # messages envoyés de la forme : "Client_X|taille_msg|cle_AES|vect_AES|message_chiffre"
        # On renomme les indices que l'on utilise dans le code pour pouvoir les modifier plus facilement
        # et pour plus de visibilité
        self.i_client = 0
        self.i_taille_msg = 1
        self.i_cle_AES = 2
        self.i_vect_AES = 3
        self.i_msg_chiffre = 4