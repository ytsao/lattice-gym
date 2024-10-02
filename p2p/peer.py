from typing import Tuple
import socket 
import threading
import random

SERVER_PORT: int = 1234
CLIENT_PORT: int = random.randint(5000, 9000)
SERVER_NAME: str = "localhost"
SERVER_ADDRESS: Tuple[str, int] = (SERVER_NAME, SERVER_PORT)
CLIENT_ADDRESS: Tuple[str, int] = (SERVER_NAME, CLIENT_PORT)
DATA_FORMAT: str = "utf-8"
DISCONNECTION_INFO: str = "!DISCONNECT"


client: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
client.bind(CLIENT_ADDRESS)


def receive():
    while True:
        try:
            message, _ = client.recvfrom(1024)
            print(f"{message.decode(DATA_FORMAT)}")
        except Exception as e:
            print(f"ERROR: {e}")

def start():
    answer: str = input("Would you like to connect (yes/no)? ")
    if answer.lower() != "yes":
        return 

    name: str = input("Enter your name: ")

    # connect to server
    client.sendto(f"SIGNUP_TAG:{name}".encode(DATA_FORMAT), SERVER_ADDRESS)
    # t2 = threading.Thread(target=receive, args=(client,))
    # t2.start()
    connection: str = ""
    port: str = ""
    while True:
        message = client.recv(1024).decode(DATA_FORMAT)
        if message.startswith("INFO:"):
            print(message)
            connection, port = message.split(":")[1].split(", ")
            break
        
    # build client socket
    # source_socket: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # source_socket.bind((SERVER_NAME, int(port)))
    print(f"source_socket is connected to {connection}, {port}")
    dest_address: Tuple[str, int] = (connection, int(port))
    while True:
        message: str = input("> ")
        if message == "q":
            client.sendto(f"{name} left the room ...".encode(DATA_FORMAT), dest_address)
            break
        
        client.sendto(message.encode(DATA_FORMAT), dest_address)

        


if __name__ == "__main__":
    t1 = threading.Thread(target=start)
    t2 = threading.Thread(target=receive)
    t1.start()
    t2.start()