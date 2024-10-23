import sys
from typing import Tuple, List
from PyQt6.QtWidgets import QApplication, QWidget, QLabel, QPushButton, QVBoxLayout
import random
import socket
import threading
import time
from functools import partial


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
        self.increment_button = QPushButton()
        self.increment_button.setText("Increment")
        self.decrement_button = QPushButton()
        self.decrement_button.setText("Decrement")

        layout.addWidget(self.count_label)
        layout.addWidget(self.debug_label)
        layout.addWidget(self.increment_button)
        layout.addWidget(self.decrement_button)

        # initial value, after connection, it is going to be either 0 or 1.
        self.node_id: int = -1
        self.count_value: int = 0
        self.increment_button.clicked.connect(partial(self.operation, isAdd=True))
        self.decrement_button.clicked.connect(partial(self.operation, isAdd=False))
        
        self.start_CRDT: bool = False
        # self.dest_address: Tuple[str,int]
        self.dest_address: List[Tuple[str,int]] = []

    def request_value(self):
        # user interface function
        self.count_label.setText(f"Current value: {self.query()}")
        info: str = f"current state: ({self.query()})\n"
        self.debug_label.setText(info)

    def operation(self, isAdd: bool):
        message: str = self._prepare(isAdd=isAdd)
        self._effect(message=message)

        # broadcast m to other replicas
        for each_dest in self.dest_address:
            self.client.sendto(f"{message}: ".encode(self.DATA_FORMAT), each_dest)
        
        # update interface
        self.request_value()

    def query(self):
        return self._eval()

    def _prepare(self, isAdd: bool):
        if isAdd:
            return "increment"
        else:
            return "decrement"

    def _effect(self, message: str):
        if "increment" in message:
            self.count_value += 1
        elif "decrement" in message:
            self.count_value -= 1

    def _eval(self):
        return self.count_value

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
                    print("Got another client's information")
                elif len(message) > 0 and "increment" in message:
                    self._effect(message)
                    self.request_value()
                elif len(message) > 0 and "decrement" in message:
                    self._effect(message)
                    self.request_value()
            except Exception as e:
                self.debug_label.setText(e)

    def start(self):
        # send message to the server
        self.client.sendto(f"SIGNUP_TAG:{self.CLIENT_PORT}".encode(self.DATA_FORMAT), self.SERVER_ADDRESS)
        while not self.start_CRDT:
            if self.node_id != -1:
                self.start_CRDT = True

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MyApp()
    t1 = threading.Thread(target=window.start, daemon=True)
    t2 = threading.Thread(target=window.receive, daemon=True)
    t1.start()
    t2.start()
    window.show()
    app.exec()
