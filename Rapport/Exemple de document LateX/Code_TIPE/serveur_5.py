            elif "/S -init" in requete_client:
                if len(decomp_msg) > 2: # si le client malveillant veut envoyer une clé altérée
                    # on l'envoie au bon destinataire
                    expediteur = decomp_msg[2]
                    destinataire = int(decomp_msg[3]) - 1
                    # on supprime les infos concernant l'expéditeur et le destinataire
                    decomp_msg.pop()
                    decomp_msg.pop()
                    requete_client = fu.compression_arg_str(decomp_msg, separateur="|")
                    self.transfert(expediteur, self.clients[destinataire][1], requete_client, separateur="|")
                else: # sinon c'est la clé d'origine d'un client normal
                    # on envoie donc sa clé au client malveillant
                    self.transfert(str(id_client), self.clients[2][1], requete_client, separateur="|")
            elif id_client != 3: # si le serveur reçoit un message d'un client normal
                # on l'envoit au client malveillant
                self.transfert(str(id_client), self.clients[2][1], requete_client, separateur="|")
            else:
                # on l'envoie au bon destinataire
                expediteur = decomp_msg[4]
                destinataire = int(decomp_msg[5]) - 1
                # on supprime les infos concernant l'expéditeur et le destinataire
                decomp_msg.pop()
                decomp_msg.pop()
                requete_client = fu.compression_arg_str(decomp_msg, separateur="|")
                self.transfert(expediteur, self.clients[destinataire][1], requete_client, separateur="|")