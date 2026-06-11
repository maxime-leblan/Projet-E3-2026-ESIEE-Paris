    def reception(self, socket):
        """ Fonction qui réceptionne les messages reçus du serveur """
        try:
            while True:
                requete_serveur = socket.recv(500)
                requete_serveur = requete_serveur.decode("utf-8")
                decomp_msg = fu.decomposition_chaine(requete_serveur, separateur="|")
                id_expediteur = fu.id_client(requete_serveur)
                
                if "/S -init" in decomp_msg:  # si on reçoit une clé publique, on l'enregistre
                    self.cles_pb_RSA_autres[id_expediteur] = fu.extraction_arg_str(decomp_msg[2])
                    print("clé reçue : ", fu.extraction_arg_str(decomp_msg[2]))
                    # on envoie notre propre clé publique au bon destinataire
                    if id_expediteur == "1":
                        socket.send(self.initialisation_RSA("/S -init", separateur="|", expediteur=id_expediteur, destinataire="2").encode("utf-8"))
                    else:
                        socket.send(self.initialisation_RSA("/S -init", separateur="|", expediteur=id_expediteur, destinataire="1").encode("utf-8"))