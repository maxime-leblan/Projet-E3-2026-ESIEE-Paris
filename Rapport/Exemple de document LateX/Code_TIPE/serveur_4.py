    def reception_cm(self, client, id_client:int):
        """ Fonction qui réceptionne les requêtes des clients et qui les traite pour le modèle 3 """
        while True:
            requete_client = client.recv(500)
            requete_client = requete_client.decode("utf-8")
            decomp_msg = fu.decomposition_chaine(requete_client, separateur="|")
            print("<Client-" + str(id_client) + ">:", requete_client)
            print() # on passe une ligne pour aérer l'affichage
            if requete_client == "/S -close":
                print("CLOSE")
                self.arret = True
            elif "/S -check" in requete_client:
                if id_client == 1:
                    destinataire = 1
                else:
                    destinataire = 0
                self.transfert(str(id_client), self.clients[destinataire][1], requete_client, separateur="|")