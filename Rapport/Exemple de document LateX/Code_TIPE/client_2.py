    def initialisation_RSA(self, message: str, separateur="|"):
        """ Fonction qui crée la clé privée et la clé publique du client pour l'algo de chiffrement RSA.
        Elle est exécutée à l'initialisation de la connection avec le serveur et partage la clé publique du client
        avec un autre client connecté au serveur avec lequel il veut communiquer."""

        cles = algos.creation_cles(self.liste_nb_premiers)
        self.cle_pb_RSA = cles[0]
        self.cle_pv_RSA = cles[1]
        msg = fu.compression_arg_str([self.cle_pb_RSA[0], self.cle_pb_RSA[1]])
        msg = message + separateur + msg

        return msg

    def envoyer(self, socket):
        """ Fonction permettant d'envoyer un message à un client en passant par le serveur """
        try:
            while True:
                msg = input("-->")
                if msg == "/S -init" and not self.init_envoi:
                    socket.send(self.initialisation_RSA(msg).encode("utf-8"))
                    self.init_envoi = True
                elif msg == "/S -close":
                    socket.send(msg.encode("utf-8"))