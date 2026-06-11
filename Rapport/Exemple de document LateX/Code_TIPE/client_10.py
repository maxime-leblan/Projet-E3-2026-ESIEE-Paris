                    if hash_autre_cle != hash_cle:
                        print("La communication n'est plus sécurisée /!\ ")
                    else:
                        print("La communication est sécurisée")
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
                print("\n-->", end="")
        except ConnectionResetError:
            print("Connexion interrompue\nSi le terminal est en attente, appuyez sur Entrée")
            sys.exit()