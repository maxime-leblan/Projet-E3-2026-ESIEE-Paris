    def run(self):
        """ Fonction qui démarre et ferme le serveur """
        # création du socket pour "démarrer" le serveur
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.bind((self.host, self.port))
        print("Le serveur est démarré...")

        self.socket.listen(1) # on le met en mode récepteur

        connections = threading.Thread(target=self.connections, args=[])
        connections.start()

        execution = threading.Thread(target=self.close, args=[])
        execution.start()

        execution.join()

        for client in self.clients:
            client[1].close()
        self.socket.close()