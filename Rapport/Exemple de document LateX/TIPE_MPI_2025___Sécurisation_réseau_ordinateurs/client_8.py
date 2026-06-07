                        else: # sinon c'est l'inverse
                            destinataire = "1"
                        # On chiffre le message avec la cle et le vecteur AES
                        msg_chiffre = algos.chiffrer_AES(msg, fu.encode_ascii(cle_AES), fu.encode_ascii(vecteur_AES))
                        msg_chiffre = msg_chiffre.decode("latin-1")
                        # On chiffre la cle et le vecteur pour AES avec le RSA
                        cle_AES = algos.chiffrer_chaine_RSA(cle_AES, self.cles_pb_RSA_autres[destinataire], taille_facteur=4)
                        vecteur_AES = algos.chiffrer_chaine_RSA(vecteur_AES, self.cles_pb_RSA_autres[destinataire], taille_facteur=4)
                        # on transforme le message, la clé et le vecteur sous forme de liste en chaine de caractères
                        msg_chiffre = fu.compression_arg_str([msg_chiffre])
                        cle_AES = fu.compression_arg_str(cle_AES)
                        vecteur_AES = fu.compression_arg_str(vecteur_AES)
                        # On construit le message que l'on va envoyer contenant toutes les infos nécessaires pour le destinataire
                        msg_envoye = str(len(msg)) + "|" + cle_AES + "|" + vecteur_AES + "|" + msg_chiffre
                        msg_envoye = msg_envoye.encode("utf-8")
                        socket.send(msg_envoye)
                    except AssertionError:
                        print("Erreur lors de la tentative de chiffrement du message. Veuillez essayer à nouveau")
        except ConnectionResetError:
            print("Déconnexion du serveur...")
            sys.exit()