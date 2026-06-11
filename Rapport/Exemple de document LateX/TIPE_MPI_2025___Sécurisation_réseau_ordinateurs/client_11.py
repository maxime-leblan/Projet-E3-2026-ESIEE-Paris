class ClientMalveillant(Client_AES_RSA):
    """ Classe fille de la classe Client_AES_RSA qui initialise le client malveillant voulant lire des conversations
    sécurisées entre les autres clients connectés au même serveur, dans le cadre du modèle n°3 avec la fonction
     de hachage et le canal secret"""
    def __init__(self, id: int, liste_nb_premiers):
        Client_AES_RSA.__init__(self, id, liste_nb_premiers)
        # Compteur indiquant le nombre d'échanges de clés que le Client Malveillant a effectué avec les autres clients
        self.nb_cles_echangees = 0

    def initialisation_RSA(self, message: str, separateur="|", expediteur="", destinataire=""):
        """ Fonction qui crée la clé privée et la clé publique du client pour l'algo de chiffrement RSA.
        Elle est exécutée à l'initialisation de la connection avec le serveur et partage la clé publique du client
        avec un autre client connecté au serveur avec lequel il veut communiquer."""
        if self.cle_pv_RSA == None: # si on a pas encore créé nos clés, on le fait
            cles = algos.creation_cles(self.liste_nb_premiers)
            self.cle_pb_RSA = cles[0]
            self.cle_pv_RSA = cles[1]
        msg = fu.compression_arg_str([self.cle_pb_RSA[0], self.cle_pb_RSA[1]])
        print("clé envoyée : ", msg)
        msg = fu.compression_arg_str([msg, expediteur, destinataire], separateur="|")
        msg = message + separateur + msg
        return msg