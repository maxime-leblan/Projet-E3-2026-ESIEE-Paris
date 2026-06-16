Import("env")
import os

def skip_files(node):
    # On récupère le chemin complet du fichier que PlatformIO s'apprête à compiler
    path = node.get_path()
    nom_fichier = os.path.basename(node.get_path())
    
    # Si le nom du fichier contient "MessageManager.cpp" ou "MessageManager.hpp"
    if nom_fichier == "MessageManager.cpp" in path or nom_fichier == "MessageManager.hpp" in path or nom_fichier == "UartMessageManager.cpp" in path or nom_fichier == "UartMessageManager.hpp" in path:
        # Retourner 'None' ordonne à PlatformIO d'abandonner ce fichier
        return None
        
    # Sinon, on laisse passer le fichier normalement
    return node

# On applique ce filtre à absolument tous les fichiers ("*")
env.AddBuildMiddleware(skip_files, "*")

