                else:
                    taille_msg = int(decomp_msg[self.i_taille_msg])
                    # on reconvertit le message chiffré (str) en liste d'entiers pour pouvoir ensuite le déchiffrer
                    msg_chiffre = fu.extraction_arg_str(decomp_msg[self.i_msg_chiffre])
                    # on vérifie qui est l'expéditeur pour prendre sa clé publique correspondante
                    if id_expediteur == 1:
                        # msg_dechiffre = algos.dechiffrer_RSA(int(decomp_msg[2]), self.cle_pv_RSA, self.cle_pb_RSA[0])
                        msg_dechiffre = algos.dechiffrer_chaine_RSA(msg_chiffre, self.cle_pv_RSA, self.cle_pb_RSA[0], taille_entiers=3)
                    else:
                        msg_dechiffre = algos.dechiffrer_chaine_RSA(msg_chiffre, self.cle_pv_RSA, self.cle_pb_RSA[0], taille_entiers=3)
                    print("\n", decomp_msg[self.i_client] + ":" + msg_dechiffre)
                print("\n-->", end="")
        except ConnectionResetError:
            print("Connexion interrompue\nSi le terminal est en attente, appuyez sur Entrée")
            sys.exit()