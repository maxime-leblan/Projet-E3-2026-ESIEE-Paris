class Client_RSA:
    """ Classe permettant de simuler un client pouvant se connecter à un serveur informatique simplifié
    à l'aide du module socket, qui base le chiffrement sur l'algorithme du RSA """

    def __init__(self, id: int, liste_nb_premiers, host="localhost", port=6390):
        self.id = id
        self.host = host
        self.port = port
        self.socket = None
        self.cle_pv_RSA = None
        self.cle_pb_RSA = None
        self.cles_pb_RSA_autres = {}
        self.liste_nb_premiers = liste_nb_premiers
        self.init_envoi = False # permet de savoir si self.initialisation_RSA() a déjà été exécutée dans self.envoyer()
        self.init_recu = False  # permet de savoir si self.initialisation_RSA() a déjà été exécutée dans self.reception()
        # messages envoyés de la forme : "Client_X:taille_msg:message_chiffre"
        # On renomme les indices que l'on utilise dans le code pour pouvoir les modifier plus facilement
        # et pour plus de visibilité
        self.i_client = 0
        self.i_taille_msg = 1
        self.i_msg_chiffre = 2