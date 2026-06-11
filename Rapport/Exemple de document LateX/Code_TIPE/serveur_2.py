    def connections(self):
        """ Fonction qui gère les connexions au serveur d'éventuels clients en temps réel """
        while True:
            client, ip = self.socket.accept()
            self.clients.append((self.id_client, client, ip))
            # on lance un fil d'exécution pour chaque nouveau client afin de pouvoir recevoir ses messages
            # on choisit la bonne fonction de réception en fonction du modèle utilisé
            if self.modele != 3:
                self.threads[str(self.id_client)] = threading.Thread(target=self.reception, args=[client, self.id_client])
            else:
                self.threads[str(self.id_client)] = threading.Thread(target=self.reception_cm, args=[client, self.id_client])
            self.threads[str(self.id_client)].start()

            print("Le client ", self.id_client, " d'adresse ", ip, " s'est connecté")
            self.id_client += 1

    def transfert(self, expediteur:str, destinataire, message:str, separateur=":"):
        """ Fonction qui transfert un message entre 2 clients """
        new_msg = "<Client-" + expediteur + ">" + separateur + message
        destinataire.send(new_msg.encode("utf-8"))