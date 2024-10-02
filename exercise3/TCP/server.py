from typing import Tuple, Set
import socket
import threading


PORT: int = 1234
SERVER_NAME: str = "localhost"
ADDRESS: Tuple[str, int] = (SERVER_NAME, PORT)
DATA_FORMAT: str = "utf-8"
DISCONNECTION_INFO: str = "!DISCONNECT"


# 1. build server
# server: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server.bind(ADDRESS)

# 2. build client
clients: Set = set()
clients_lock = threading.Lock()


def handle_client(connection, address):
    print(f"new connection: {connection} is connected")
    try:
        connected: bool = True
        while connected:
            message = connection.recv(1024).decode(DATA_FORMAT)
            if not message:
                break
            if message == DISCONNECTION_INFO:
                connected = False
            print(f"[{address}] - {message}")
            with clients_lock:
                for c in clients:
                    c.sendall(f"[{address}] - {message}".encode(DATA_FORMAT))
    finally:
        with clients_lock:
            clients.remove(connection)

        connection.close()
    

def start():
    print("server is started!")
    server.listen()
    print("start is listening ...")
    while True:
        connection, address = server.accept()
        with clients_lock:
            clients.add(connection)
        thread = threading.Thread(target=handle_client, args=(connection, address))
        thread.start()

if __name__ == "__main__":
    start()