from typing import Tuple
import socket 
import time 

PORT: int = 1234
SERVER_NAME: str = "localhost"
ADDRESS: Tuple[str, int] = (SERVER_NAME, PORT)
DATA_FORMAT: str = "utf-8"
DISCONNECTION_INFO: str = "!DISCONNECT"

def connect():
    client: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(ADDRESS)
    return client

def send(client, message):
    message = message.encode(DATA_FORMAT)
    client.send(message)

def start():
    answer: str = input("Would you like to connect (yes/no)? ")
    if answer.lower() != "yes":
        return 
    
    connection = connect()
    while True:
        message = input("Message( q for quit): ")
        if message == "q":
            break
        send(connection, message)
    send(connection, DISCONNECTION_INFO)
    time.sleep(1)
    print("!disconnected")


if __name__ == "__main__":
    start()