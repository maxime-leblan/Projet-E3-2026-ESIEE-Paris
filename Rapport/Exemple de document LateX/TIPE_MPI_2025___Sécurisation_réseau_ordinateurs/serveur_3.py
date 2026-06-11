    def reception(self, client, id_client:int):
        """ Fonction qui réceptionne les requêtes des clients et qui les traite pour les modèles 1 et 2"""
        while True:
            requete_client = client.recv(500)
            requete_client = requete_client.decode("utf-8")
            print("<Client-" + str(id_client) + ">:", requete_client)
            print() # on passe une ligne pour aérer l'affichage
            if requete_client == "/S -close":
                print("CLOSE")
                self.arret = True
            else: # on envoie le message au bon destinataire en fonction de l'expéditeur
                if id_client == 1:
                    self.transfert(str(id_client), self.clients[1][1], requete_client, separateur="|")
                else:
                    self.transfert(str(id_client), self.clients[0][1], requete_client, separateur="|")