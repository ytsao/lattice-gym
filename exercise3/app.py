import sys
from typing import Tuple
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
        self.count_label.setText("Hello, just for testing")
        self.debug_label = QLabel()
        self.debug_label.setText("Debugging Information")
        self.push_button = QPushButton()
        self.push_button.setText("Click me : )")

        layout.addWidget(self.count_label)
        layout.addWidget(self.debug_label)
        layout.addWidget(self.push_button)

        # initial value, after connection, it is going to be either 0 or 1.
        self.node_id: int = -1
        self.count: list = [0, 0]
        self.local_count: int = 0
        self.push_button.clicked.connect(self.increment)
        self.start_CRDT: bool = False
        self.dest_address: Tuple[str,int]

    def request_value(self):
        # user interface function
        self.count_label.setText(f"Current value: {self._value()}")

    def increment(self):
        # user interface function
        self._update()

    def _update(self):
        # CRDT implementation -> user interface : increment
        # increments at vector index corresponding to local node id
        if self.node_id != -1:
            self.count[self.node_id] += 1
        else:
            print("there is only 1 client")

    def _merge(self, other_count: list):
        # called asynchronously
        # coordinatewise max
        for i in range(len(self.count)):
            self.count[i] = max(self.count[i], other_count[i])

    def _value(self):
        # CRDT implementation -> user interface : value
        # sum all Ints in vector
        return sum(self.count)

    def receive(self):
        # receive count vector from another client
        # call merge function
        while True:
            try:
                message, _ = self.client.recvfrom(1024)
                message = message.decode(self.DATA_FORMAT)
                self.debug_label.setText(message)
                if len(message) > 0 and "state_vector: " in message:
                    first_state, second_state = message[message.index(":")+1:].split(",")
                    print(f"first_state: {first_state}, second_state: {second_state}")
                    state_vector_from_another: list = [int(first_state), int(second_state)]
                    self._merge(other_count=state_vector_from_another)

                    # update count label
                    self.request_value()
                elif message.startswith("INFO:"):
                    str_node_id, connection, port = message.split(":")[1].split(", ")
                    self.node_id = int(str_node_id)
                    self.dest_address = (connection, int(port))
                    print("Got another client's information")
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
        while True:
            time.sleep(10)
            self.client.sendto(f"state_vector: {self.count[0]},{self.count[1]}".encode(self.DATA_FORMAT), self.dest_address)




if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MyApp()
    t1 = threading.Thread(target=window.start, daemon=True)
    t2 = threading.Thread(target=window.receive, daemon=True)
    t1.start()
    t2.start()
    window.show()
    app.exec()
