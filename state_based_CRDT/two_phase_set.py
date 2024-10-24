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
        self.A: Set[str] = set()
        self.R: Set[str] = set()
        
        self.start_CRDT: bool = False
        self.dest_address: List[Tuple[str,int]] = []
                
        self.add_button.clicked.connect(partial(self.update, isAdd=True))
        self.remove_button.clicked.connect(partial(self.update, isAdd=False)) 
        self.lookup_button.clicked.connect(self.lookup)


    def request_value(self):
        # user interface function
        self.state_label.setText(f"Current State: {self._value()}\n A: {self.A}\n R: {self.R}")

    def update(self, isAdd: bool):
        # user interface function
        if isAdd: 
            self._add()
            self.add_text.clear()
        else: 
            self._remove()
            self.remove_text.clear()
        
        self.request_value()
        
    def lookup(self):
        # user interface function
        lookup_value: str = self.lookup_text.text().strip()
        lookup_result: bool = self._lookup(lookup_value)
        
        if lookup_result:
            self.info_label.setText(f"{lookup_value} is in current state.")
        else:
            self.info_label.setText(f"{lookup_value} is not in current state.")

    def _add(self):
        # CRDT implementation
        add_value: str = self.add_text.text().strip()
        if add_value != "Enter the element to add":
            self.A.add(add_value)

    def _remove(self):
        # CRDT implementation
        remove_value: str = self.remove_text.text().strip()
        if remove_value != "Enter the element to remove":
            if self._lookup(remove_value):
                self.R.add(remove_value)
    
    def _lookup(self, lookup_value: str):
        # CRDT implementation
        isAdded: bool = lookup_value in self.A
        isNotRemoved: bool = lookup_value not in self.R

        return isAdded and isNotRemoved

    def _merge(self, other_set: set, isAdd: bool):
        # called asynchronously
        if isAdd:
            self.A = self.A.union(other_set)
        else:
            self.R = self.R.union(other_set)

    def _value(self):
        # CRDT implementation -> user interface : value
        return self.A - self.R
   
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
                elif len(message) > 0 and "Added set:" in message:
                    str_added_elements: str = message.split(":")[1]
                    set_added_elements: Set[str] = set(str_added_elements.split(","))
                    self._merge(other_set=set_added_elements, isAdd=True)
                elif len(message) > 0 and "Removed set:" in message:
                    str_removed_elements: str = message.split(":")[1]
                    set_removed_elements: Set[str] = set(str_removed_elements.split(","))
                    self._merge(other_set=set_removed_elements, isAdd=False)
                
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

        # connect with another client
        # send the "state_vector" to another client every 10 seconds
        print("start CRDT")
        while True:
            time.sleep(5)
            str_added_elements: str = ",".join(self.A)
            str_removed_elements: str = ",".join(self.R)

            # send to all others 
            for each_dest_address in self.dest_address:
                if len(str_added_elements) != 0:
                    self.client.sendto(f"Added set:{str_added_elements}".encode(self.DATA_FORMAT), each_dest_address)
                if len(str_removed_elements) != 0:
                    self.client.sendto(f"Removed set:{str_removed_elements}".encode(self.DATA_FORMAT), each_dest_address)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MyApp()
    t1 = threading.Thread(target=window.start, daemon=True)
    t2 = threading.Thread(target=window.receive, daemon=True)
    t1.start()
    t2.start()
    window.show()
    app.exec()