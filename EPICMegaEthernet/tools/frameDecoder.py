import sys
import socket

sys.path.append('./protopy')    

import epicethernetoutput_pb2

def receive_protobuf_message(port):
    # Créez un objet socket pour le serveur TCP
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Attachez le socket à l'adresse et au port spécifiés
    server_socket.bind(('127.0.0.1', port))

    # Mettez le socket en mode écoute
    server_socket.listen(1)

    print(f"En attente de connexion sur le port {port}...")
    conn, addr = server_socket.accept()
    print(f"Connexion établie avec {addr}")

    # Recevez les données depuis le client
    received_data = conn.recv(4096)  # 4096 est la taille du tampon pour les données, ajustez si nécessaire

    # Fermez la connexion
    conn.close()
    server_socket.close()

    # Désérialisez le message protobuf reçu
    listout=epicethernetoutput_pb2.EpicEthernetOutput();
    listout.ParseFromString(received_data)

    return listout

# Exemple d'utilisation :
server_port = 4200  # Le port sur lequel le serveur TCP doit écouter
received_message = receive_protobuf_message(server_port)
print("Message reçu:")
print(received_message.nbChannel)
# Accédez aux champs du message reçu :
for dig_output in received_message.digoutputs:
    print(f"DigitalOutput id: {dig_output.numChannel}, value: {dig_output.value}")
# Ajoutez ici l'affichage ou le traitement des autres champs de votre message
