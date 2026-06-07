    def envoyer(self, socket):
        """ Fonction permettant d'envoyer un message à un client en passant par le serveur """
        try:
            while True:
                msg = input("-->")
                if msg == "/S -init" and not self.init_envoi:
                    # on envoie notre clé publique à l'autre client
                    socket.send(self.initialisation_RSA(msg, separateur="|").encode("utf-8"))
                    self.init_envoi = True
                elif msg == "/S -close":
                    socket.send(msg.encode("utf-8"))
                elif msg == "/S -check":
                    # on construit le hash de la clé publique de l'expéditeur et on l'envoie à l'autre client
                    cle_pb = str(self.cle_pb_RSA[0]) + "/" + str(self.cle_pb_RSA[1])
                    hash_cle = hashlib.sha256(cle_pb.encode())
                    hash_cle = hash_cle.hexdigest()
                    msg += "|" + hash_cle
                    socket.send(msg.encode())
                else:
                    try:
                        cle_AES = fu.generateur_alea_chaine(32)
                        vecteur_AES = fu.generateur_alea_chaine(16)
                        # si on veut envoyer un msg au client 1, on chiffre avec la clé publique du client 2
                        if self.id == 1:  # on chiffre le message avec la clé publique du bon client
                            destinataire = "2"