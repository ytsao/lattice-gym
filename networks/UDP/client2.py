from typing import Tuple
import socket
import threading
import sys


PORT: int = 5052
SERVER_NAME: str = "localhost"
ADDRESS: Tuple[str, int] = (SERVER_NAME, PORT)
DATA_FORMAT: str = "utf-8"


rendezvous: Tuple[str, int] = (SERVER_NAME, 5050)

# connect to redezvous
print("connecting to redezvous server")

client: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
client.bind(ADDRESS)
client.sendto(b'0', rendezvous)


while True:
    message = client.recv(1024).decode(DATA_FORMAT)
    if message.strip() == "ready":
        print("checked in with server, waiting")
        break

message = client.recv(1024).decode(DATA_FORMAT)
ip, source_port, dest_port = message.split(' ')
source_port = int(source_port)
dest_port = int(dest_port)

print("\n got peer")
print(f"    ip:    {ip}")
print(f"    source port: {source_port}")
print(f"    dest_port:  {dest_port} \n")
client.close()

# punch hole
print("punching hole")
source_socket: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
source_socket.bind((SERVER_NAME, source_port))
source_socket.sendto(b"0", (ip, dest_port))
source_socket.close()

print("ready to exchange messages \n")

def listen():
    source_socket: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    source_socket.bind((SERVER_NAME, source_port))

    while True:
        message = source_socket.recv(1024)
        print(f"\rpeer: {message.decode(DATA_FORMAT)} \n> ", end='')

listener = threading.Thread(target=listen,daemon=True)
listener.start()

dest_socket: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
dest_socket.bind((SERVER_NAME, dest_port))
while True:
    message: str = input("> ")
    dest_socket.sendto(message.encode(DATA_FORMAT), (ip, source_port))