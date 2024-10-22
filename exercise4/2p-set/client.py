import sys
from typing import Tuple, List, Set
from PyQt6.QtWidgets import QApplication, QWidget, QLabel, QPushButton, QVBoxLayout, QLineEdit
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
        self.state_label = QLabel()
        self.state_label.setText("Current State: ")
        self.info_label = QLabel()
        self.info_label.setText("Information: ")

        self.add_text = QLineEdit()
        self.add_text.setText("Enter the element to add")
        self.add_button = QPushButton()
        self.add_button.setText("Add")
        self.remove_text = QLineEdit()
        self.remove_text.setText("Enter the element to remove") 
        self.remove_button = QPushButton()
        self.remove_button.setText("Remove")
        self.lookup_text = QLineEdit()
        self.lookup_text.setText("Enter the element to lookup")
        self.lookup_button = QPushButton()
        self.lookup_button.setText("Lookup")

        layout.addWidget(self.state_label)
        layout.addWidget(self.info_label)

        layout.addWidget(self.add_text)
        layout.addWidget(self.remove_text)
        layout.addWidget(self.add_button)
        layout.addWidget(self.remove_button)
        layout.addWidget(self.lookup_text)
        layout.addWidget(self.lookup_button)

        self.add_button.clicked.connect(self.add)
        self.remove_button.clicked.connect(self.remove) 
        self.lookup_button.clicked.connect(self.lookup)

        # initial value, after connection, it is going to be either 0 or 1.
        self.node_id: int = -1
        self.A: Set[str] = set()
        self.R: Set[str] = set()
        
        self.start_CRDT: bool = False
        self.dest_address: List[Tuple[str,int]] = []

    def request_value(self):
        # user interface function
        self.state_label.setText(f"Current State: {self._value()}")
        self.info_label.setText(f"Information: A: {self.A}, R: {self.R}")

    def add(self):
        # user interface function
        add_value: str = self.add_text.text()
        if add_value != "" or add_value != "Enter the element to add":
            self.A.add(self.add_text.text())

    def lookup(self):
        # user interface function
        lookup_value: str = self.lookup_text.text()
        return self._lookup(lookup_value)

    def _lookup(self, lookup_value: str):
        # CRDT implementation -> user interface : lookup
        isAdded: bool = lookup_value in self.A
        isNotRemoved: bool = lookup_value not in self.R

        return isAdded and isNotRemoved

    def remove(self):
        # user interface function
        remove_value: str = self.remove_text.text()
        if remove_value != "" or remove_value != "Enter the element to remove":
            if self._lookup(remove_value):
                self.R.add(remove_value)

    # def _update(self, isIncrement: bool):
    #     # CRDT implementation -> user interface : increment
    #     # increments at vector index corresponding to local node id
    #     if self.node_id != -1:
    #         if isIncrement:
    #             self.p_state_vector[self.node_id] += 1
    #         else:
    #             self.n_state_vector[self.node_id] += 1
    #     else:
    #         print("there is only 1 client")

    # def _merge(self, other_state_vector: list, isIncrement: bool):
    #     # called asynchronously
    #     # coordinatewise max
    #     if isIncrement:
    #         for i in range(len(self.p_state_vector)):
    #             self.state_vector[i] = max(self.p_state_vector[i], other_state_vector[i])
    #     else:
    #         for i in range(len(self.n_state_vector)):    
    #             self.state_vector[i] = max(self.n_state_vector[i], other_state_vector[i])

    def _value(self):
        # CRDT implementation -> user interface : value
        # sum all Ints in vector
        return self.A - self.R

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
                    self.p_state_vector.append(0)
                    self.n_state_vector.append(0)
                    print("Got another client's information")
                elif len(message) > 0 and "p_state_vector: " in message:
                    list_str_sv: List[str] = message[message.index(":")+1:].split(",")
                    state_vector_from_another: list = [int(x) for x in list_str_sv]
                    self._merge(other_state_vector=state_vector_from_another, isIncrement=True)
                elif len(message) > 0 and "n_state_vector: " in message:
                    list_str_sv: List[str] = message[message.index(":")+1:].split(",")
                    state_vector_from_another: list = [int(x) for x in list_str_sv]
                    self._merge(other_state_vector=state_vector_from_another, isIncrement=False)
                
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
        while True:
            time.sleep(5)
            # self.client.sendto(f"state_vector: {self.state_vector[0]},{self.state_vector[1]}".encode(self.DATA_FORMAT), self.dest_address)
            psv: str = ",".join([str(i) for i in self.p_state_vector])
            nsv: str = ",".join([str(i) for i in self.n_state_vector])

            # send to all others 
            for each_dest_address in self.dest_address:
                self.client.sendto(f"p_state_vector: {psv}".encode(self.DATA_FORMAT), each_dest_address)
                self.client.sendto(f"n_state_vector: {nsv}".encode(self.DATA_FORMAT), each_dest_address)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MyApp()
    t1 = threading.Thread(target=window.start, daemon=True)
    t2 = threading.Thread(target=window.receive, daemon=True)
    t1.start()
    t2.start()
    window.show()
    app.exec()
