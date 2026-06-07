    def reception(self, socket):
        """ Fonction qui réceptionne les messages reçus du serveur """
        try:
            while True:
                requete_serveur = socket.recv(500)
                requete_serveur = requete_serveur.decode("utf-8")
                # requete_serveur est de la forme : "Client_X:taille_msg:message_chiffre"
                decomp_msg = fu.decomposition_chaine(requete_serveur)
                id_expediteur = fu.id_client(requete_serveur)

                if "/S -init" in decomp_msg and not self.init_recu: # si on a pas enregistré les clés publiques de l'autre client, on le fait
                    self.cles_pb_RSA_autres[id_expediteur] = fu.extraction_arg_str(decomp_msg[self.i_msg_chiffre])
                    self.init_recu = True
                    if not self.init_envoi: # si on a pas échangé notre clé publique, on le fait
                        socket.send(self.initialisation_RSA("/S -init").encode("utf-8"))
                        self.init_envoi = True