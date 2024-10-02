from typing import Tuple, List
import socket
import threading
import queue

PORT: int = 5050
SERVER_NAME: str = "localhost"
ADDRESS: Tuple[str, int] = (SERVER_NAME, PORT)
DATA_FORMAT: str = "utf-8"

server: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server.bind(ADDRESS)
messages = queue.Queue()
clients = []


def receive():
    while True:
        try:
            message, client_address = server.recvfrom(1024)
            messages.put((message, client_address))
        except:
            pass


def broadcast():
    while True:
        while not messages.empty():
            message, client_address = messages.get()
            print(message.decode(DATA_FORMAT))
            if client_address not in clients:
                clients.append(client_address)
            for each_client in clients:
                try:
                    if message.decode(DATA_FORMAT).startswith("SIGNUP_TAG:"):
                        name = message.decode(DATA_FORMAT)[
                            message.decode(DATA_FORMAT).index(":")+1:]
                        server.sendto(f"{name} joined!".encode(
                            DATA_FORMAT), each_client)
                    else:
                        server.sendto(message, each_client)
                except Exception as e:
                    print(f"ERROR: \n {e}")
                    clients.remove(each_client)


t1 = threading.Thread(target=receive)
t2 = threading.Thread(target=broadcast)
t1.start()
t2.start()
