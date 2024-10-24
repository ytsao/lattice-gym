from typing import Tuple
import socket
import threading
import sys
import random


PORT: int = random.randint(5000, 7000)
SERVER_NAME: str = "localhost"
ADDRESS: Tuple[str, int] = (SERVER_NAME, PORT)
DATA_FORMAT: str = "utf-8"


client: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
client.bind(ADDRESS)
name = input("name: ")


def receive():
    while True:
        try:
            message, _ = client.recvfrom(1024)
            print(message.decode(DATA_FORMAT))
        except:
            pass


t = threading.Thread(target=receive)
t.start()
client.sendto(f"SIGNUP_TAG: {name}".encode(DATA_FORMAT), (SERVER_NAME, 5050))
while True:
    message = input("")
    if message == "q":
        break
    else:
        client.sendto(f"{name}: {message}".encode(
            DATA_FORMAT), (SERVER_NAME, 5050))
