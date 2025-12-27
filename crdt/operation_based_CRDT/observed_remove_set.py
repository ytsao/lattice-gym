import sys
from typing import Tuple, List, Set
from PyQt6.QtWidgets import QApplication, QWidget, QLabel, QPushButton, QVBoxLayout, QLineEdit
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
        self.state_label = QLabel()
        self.state_label.setText("Current State: ")
        self.info_label = QLabel()
        self.info_label.setText("Information: ")

        self.add_text = QLineEdit()
        self.add_text.setPlaceholderText("Enter the element to add")
        self.add_button = QPushButton()
        self.add_button.setText("Add")
        self.remove_text = QLineEdit()
        self.remove_text.setPlaceholderText("Enter the element to remove") 
        self.remove_button = QPushButton()
        self.remove_button.setText("Remove")
        self.lookup_text = QLineEdit()
        self.lookup_text.setPlaceholderText("Enter the element to lookup")
        self.lookup_button = QPushButton()
        self.lookup_button.setText("Lookup")

        layout.addWidget(self.state_label)
        layout.addWidget(self.info_label)

        layout.addWidget(self.add_text)
        layout.addWidget(self.add_button)
        layout.addWidget(self.remove_text)
        layout.addWidget(self.remove_button)
        layout.addWidget(self.lookup_text)
        layout.addWidget(self.lookup_button)

        # initial value, after connection, it is going to be either 0 or 1.
        self.node_id: int = -1
        self.AT: Set[Tuple[Tuple[str,int],str]] = set()
        self.operation_idx: int = 0
        
        self.start_CRDT: bool = False
        self.dest_address: List[Tuple[str,int]] = []
                
        self.add_button.clicked.connect(partial(self.operation, isAdd=True))
        self.remove_button.clicked.connect(partial(self.operation, isAdd=False)) 
        self.lookup_button.clicked.connect(self.lookup)


    def request_value(self):
        # user interface function
        self.state_label.setText(f"Current State: {self._eval()}\n AT: {self.AT}")

    def operation(self, isAdd: bool):
        x: str = ""
        if isAdd:
            x = self.add_text.text()
            self.add_text.clear()
            self.operation_idx += 1
        else:
            x = self.remove_text.text()
            self.remove_text.clear()
        
        message: str = self._prepare(x=x, isAdd=isAdd)
        self._effect(message=message)

        # broadcast m to other replicas
        for each_dest in self.dest_address:
            self.client.sendto(f"{message}".encode(self.DATA_FORMAT), each_dest)
        
        # update interface
        self.request_value()

    def query(self):
        return self._eval()
    
    def lookup(self):
        # user interface function
        lookup_value: str = self.lookup_text.text().strip()
        lookup_result: bool = self._lookup(lookup_value)
        
        if lookup_result:
            self.info_label.setText(f"{lookup_value} is in current state.")
        else:
            self.info_label.setText(f"{lookup_value} is not in current state.")

    def _prepare(self, x: str, isAdd: bool):
        message: str = ""
        if isAdd:
            message = f"add:{self.node_id},{self.operation_idx},{x}"
        else:
            message = f"remove:{self.node_id},{self.operation_idx},{x}"
        
        self.info_label.setText(message)

        return message

    def _effect(self, message: str):
        node_id, operation_idx, x = message.split(":")[1].split(",")
        uid = (int(node_id), int(operation_idx))
        x = (uid, x)
        if "add" in message:
            self.AT.add(x)
        elif "remove" in message:
            R = set(a for a in self.AT if a[1] == x[1])
            self.AT = self.AT - R
        
    def _lookup(self, lookup_value: str):
        value = self._eval()
        return lookup_value in value

    def _eval(self):
        return set(a[1] for a in self.AT)

    def receive(self):
        # receive "state_vector" from another client
        # call merge function
        while True:
            try:
                message, _ = self.client.recvfrom(1024)
                message = message.decode(self.DATA_FORMAT)
                self.info_label.setText(message)
                if message.startswith("INFO:"):
                    str_node_id, connection, port = message.split(":")[1].split(", ")
                    self.node_id = int(str_node_id)
                    self.dest_address.append((connection, int(port)))
                    print("Got another client's information")
                elif len(message) > 0 and "add:" in message:
                    self._effect(message=message)
                
                    # update count label
                    self.request_value()
                elif len(message) > 0 and "remove:" in message:
                    self._effect(message=message)
                
                    # update count label
                    self.request_value()
            except Exception as e:
                self.info_label.setText(e)

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
