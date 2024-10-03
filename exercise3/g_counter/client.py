import sys
from typing import Tuple, List
from PyQt6.QtWidgets import QApplication, QWidget, QLabel, QPushButton, QVBoxLayout
import random
import socket
import threading
import time


class MyApp(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Lattice Gym")
        self.resize(300, 300)  # width, height
        layout = QVBoxLayout()
        self.setLayout(layout)

        # socket parameters
        self.SERVER_PORT: int = 1234
        self.CLIENT_PORT: int = random.randint(5000, 9000)
        self.SERVER_NAME: str = "localhost"
        self.SERVER_ADDRESS: Tuple[str, int] = (self.SERVER_NAME, self.SERVER_PORT)
        self.CLIENT_ADDRESS: Tuple[str, int] = (self.SERVER_NAME, self.CLIENT_PORT)
        self.DATA_FORMAT: str = "utf-8"
        self.DISCONNECTION_INFO: str = "!DISCONNECT"

        self.client: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.client.bind(self.CLIENT_ADDRESS)

        # widgets
        self.count_label = QLabel()
        self.count_label.setText("Hello, Welcome to lattice gym!")
        self.debug_label = QLabel()
        self.debug_label.setText("Debugging Information")
        self.push_button = QPushButton()
        self.push_button.setText("Click me : )")

        layout.addWidget(self.count_label)
        layout.addWidget(self.debug_label)
        layout.addWidget(self.push_button)

        # initial value, after connection, it is going to be either 0 or 1.
        self.node_id: int = -1
        self.state_vector: List[int] = [0]
        self.push_button.clicked.connect(self.increment)
        self.start_CRDT: bool = False
        # self.dest_address: Tuple[str,int]
        self.dest_address: List[Tuple[str,int]] = []

    def request_value(self):
        # user interface function
        self.count_label.setText(f"Current value: {self._value()}")
        sv: str = ",".join([str(i) for i in self.state_vector])
        self.debug_label.setText(f"{sv}")

    def increment(self):
        # user interface function
        self._update()

    def _update(self):
        # CRDT implementation -> user interface : increment
        # increments at vector index corresponding to local node id
        if self.node_id != -1:
            self.state_vector[self.node_id] += 1
        else:
            print("there is only 1 client")

    def _merge(self, other_state_vector: list):
        # called asynchronously
        # coordinatewise max
        for i in range(len(self.state_vector)):
            self.state_vector[i] = max(self.state_vector[i], other_state_vector[i])

    def _value(self):
        # CRDT implementation -> user interface : value
        # sum all Ints in vector
        return sum(self.state_vector)

    def receive(self):
        # receive "state_vector" from another client
        # call merge function
        while True:
            try:
                message, _ = self.client.recvfrom(1024)
                message = message.decode(self.DATA_FORMAT)
                self.debug_label.setText(message)
                if message.startswith("INFO:"):
                    str_node_id, connection, port = message.split(":")[1].split(", ")
                    self.node_id = int(str_node_id)
                    self.dest_address.append((connection, int(port)))
                    self.state_vector.append(0)
                    print("Got another client's information")
                elif len(message) > 0 and "state_vector: " in message:
                    list_str_sv: List[str] = message[message.index(":")+1:].split(",")
                    state_vector_from_another: list = [int(x) for x in list_str_sv]
                    self._merge(other_state_vector=state_vector_from_another)

                    # update count label
                    self.request_value()
            except Exception as e:
                self.debug_label.setText(e)

    def start(self):
        # send message to the server
        self.client.sendto(f"SIGNUP_TAG:{self.CLIENT_PORT}".encode(self.DATA_FORMAT), self.SERVER_ADDRESS)
        while not self.start_CRDT:
            if self.node_id != -1:
                self.start_CRDT = True

        # connect with another client
        # send the "state_vector" to another client every 10 seconds
        print("start CRDT")
        print("jfioajdf oijdaoijf aoij oia")
        while True:
            time.sleep(5)
            # self.client.sendto(f"state_vector: {self.state_vector[0]},{self.state_vector[1]}".encode(self.DATA_FORMAT), self.dest_address)
            sv: str = ",".join([str(i) for i in self.state_vector])
            for each_dest_address in self.dest_address:
                self.client.sendto(f"state_vector: {sv}".encode(self.DATA_FORMAT), each_dest_address)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MyApp()
    t1 = threading.Thread(target=window.start, daemon=True)
    t2 = threading.Thread(target=window.receive, daemon=True)
    t1.start()
    t2.start()
    window.show()
    app.exec()
