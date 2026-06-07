                else:
                    # On extrait et déchiffre la clé et le vecteur AES
                    cle_AES = fu.extraction_arg_str(decomp_msg[self.i_cle_AES])
                    cle_AES = algos.dechiffrer_chaine_RSA(cle_AES, self.cle_pv_RSA, self.cle_pb_RSA[0], taille_entiers=4)
                    vecteur_AES = fu.extraction_arg_str(decomp_msg[self.i_vect_AES])
                    vecteur_AES = algos.dechiffrer_chaine_RSA(vecteur_AES, self.cle_pv_RSA, self.cle_pb_RSA[0], taille_entiers=4)

                    # on récupère le message chiffré et on le déchiffre
                    msg_chiffre = decomp_msg[self.i_msg_chiffre]
                    msg_chiffre = msg_chiffre.encode("latin-1")
                    msg_dechiffre = algos.dechiffrer_AES(msg_chiffre, fu.encode_ascii(cle_AES), fu.encode_ascii(vecteur_AES))
                    print("\n", decomp_msg[self.i_client] + ":" + msg_dechiffre)

                    # on renvoie le message au destinataire chiffré avec la clé publique du destinataire
                    # si on veut envoyer un msg au client 1, on chiffre avec la clé publique du client 2
                    if id_expediteur == "1":  # on chiffre le message avec la clé publique du bon client
                        destinataire = "2"
                    else:  # sinon c'est l'inverse
                        destinataire = "1"