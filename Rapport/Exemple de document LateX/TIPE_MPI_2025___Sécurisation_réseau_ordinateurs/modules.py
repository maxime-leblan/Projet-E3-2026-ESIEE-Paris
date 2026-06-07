# Imoportation de modules auxiliaires de Python
import random as rd
import time
import threading
import socket
import sys
# Module utilisé pour l'utilisation d'une fonction de hachage
import hashlib
# Modules nécessaires pour l'implémentation d'AES avec des bibliothèques
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import padding
# Importation d'autres fichiers Python
import fonctions_utiles as fu
import algos_chiffrement as algos