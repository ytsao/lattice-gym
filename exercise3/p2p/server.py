from typing import Tuple, List
import socket
import threading
import queue
import sys
import time 

PORT: int = 1234
SERVER_NAME: str = "localhost"
ADDRESS: Tuple[str, int] = (SERVER_NAME, PORT)
DATA_FORMAT: str = "utf-8"

server: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server.bind(ADDRESS)
messages = queue.Queue()
clients: List = []


def receive():
    while True:
        try:
            message, client_address = server.recvfrom(1024)
            messages.put((message, client_address))
        except:
            pass


def broadcast():
    while True:
        while True:
            message, client_address = messages.get()
            print(f"message: {message.decode(DATA_FORMAT)}")
            print(f"client_address: {client_address}")
            if message.decode(DATA_FORMAT).startswith("SIGNUP_TAG:"):
                name = message.decode(DATA_FORMAT)[message.decode(DATA_FORMAT).index(":")+1:]
                server.sendto(f"{name} joined!".encode(DATA_FORMAT), client_address)
                clients.append(client_address)
            if len(clients) == 2:
                print("there are two clients")
                break


        time.sleep(3)
        # send to client 1
        # send to client 2
        print("send connection information to both clients")
        client1_info: str = f"INFO:{clients[1][0]}, {clients[1][1]}"
        client2_info: str = f"INFO:{clients[0][0]}, {clients[0][1]}"
        server.sendto(client1_info.encode(DATA_FORMAT), clients[0])
        server.sendto(client2_info.encode(DATA_FORMAT), clients[1])
        print("server finished its jobs, terminate")
        # sys.exit()
        # break


t1 = threading.Thread(target=receive)
t2 = threading.Thread(target=broadcast)
t1.start()
t2.start()
print("server is starting ...")
