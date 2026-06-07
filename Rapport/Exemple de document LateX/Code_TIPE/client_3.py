                else:
                    try:
                        if self.id == 1: # on chiffre le message avec la clé publique du bon client
                            # msg_chiffre = algos.chiffrer_RSA(algos.string_to_int(msg), self.cles_pb_RSA_autres["2"])
                            msg_chiffre = algos.chiffrer_chaine_RSA(msg, self.cles_pb_RSA_autres["2"], taille_facteur=3)
                        else:
                            # msg_chiffre = algos.chiffrer_RSA(algos.string_to_int(msg), self.cles_pb_RSA_autres["1"])
                            msg_chiffre = algos.chiffrer_chaine_RSA(msg, self.cles_pb_RSA_autres["1"], taille_facteur=3)
                        # on transforme le message sous forme de liste en chaine de caractères
                        msg_chiffre = fu.compression_arg_str(msg_chiffre)
                        msg_envoye = str(len(msg)) + "|" + str(msg_chiffre) # on ajoute la longueur du message chiffré au message envoyé
                        msg_envoye = msg_envoye.encode("utf-8")
                        socket.send(msg_envoye)
                    except AssertionError:
                        print("Erreur lors de la tentative de chiffrement du message. Veuillez essayer à nouveau")
        except ConnectionResetError:
            print("Déconnexion du serveur...")
            sys.exit()