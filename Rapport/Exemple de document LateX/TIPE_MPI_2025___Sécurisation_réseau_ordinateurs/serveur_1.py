class Serveur:
    """ Classe permettant de simuler un serveur informatique simplifié à l'aide du module socket """

    def __init__(self, modele:int, port=6390):
        self.host = ""
        self.port = port
        self.socket = None
        self.clients = []
        self.threads = {}
        self.id_client = 1
        self.arret = False
        self.modele = modele

    def close(self):
        """ Fonction qui vérifie si le serveur doit s'éteindre ou non """
        while True:
            if self.arret == True:
                break