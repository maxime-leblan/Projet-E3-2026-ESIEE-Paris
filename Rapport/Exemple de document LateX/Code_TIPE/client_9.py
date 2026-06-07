    def reception(self, socket):
        """ Fonction qui réceptionne les messages reçus du serveur """
        try:
            while True:
                requete_serveur = socket.recv(500)
                requete_serveur = requete_serveur.decode("utf-8")
                decomp_msg = fu.decomposition_chaine(requete_serveur, separateur="|")
                id_expediteur = fu.id_client(requete_serveur)
                if "/S -init" in decomp_msg and not self.init_recu:  # si on a pas enregistré les clés publiques de l'autre client, on le fait
                    self.cles_pb_RSA_autres[id_expediteur] = fu.extraction_arg_str(decomp_msg[2])
                    self.init_recu = True
                    if not self.init_envoi:  # si on a pas échangé notre clé publique, on le fait
                        socket.send(self.initialisation_RSA("/S -init", separateur="|").encode("utf-8"))
                        self.init_envoi = True
                elif "/S -check" in requete_serveur:
                    print("\nComparaison des clés en cours...")
                    # on construit le hash de la clé de l'autre client à partir de notre base de donnée
                    cle_pb = str(self.cles_pb_RSA_autres[id_expediteur][0]) + "/" + str(self.cles_pb_RSA_autres[id_expediteur][1])
                    hash_cle = hashlib.sha256(cle_pb.encode())
                    hash_cle = hash_cle.hexdigest()
                    # on récupère le hash envoyé par l'autre client
                    hash_autre_cle = decomp_msg[-1]