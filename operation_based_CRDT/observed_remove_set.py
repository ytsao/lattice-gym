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
        self.A: Set[Tuple[Tuple[str,int],str]] = set()
        self.T: Set[Tuple[Tuple[str,int],str]] = set()
        self.operation_idx: int = 0
        
        self.start_CRDT: bool = False
        self.dest_address: List[Tuple[str,int]] = []
                
        self.add_button.clicked.connect(partial(self.operation, isAdd=True))
        self.remove_button.clicked.connect(partial(self.operation, isAdd=False)) 
        self.lookup_button.clicked.connect(self.lookup)


    def request_value(self):
        # user interface function
        self.state_label.setText(f"Current State: {self._value()}\n A: {self.A}\n T: {self.T}")

    def operation(self, isAdd: bool):
        message: str = self._prepare(isAdd=isAdd)
        self._effect(message=message)

        # broadcast m to other replicas
        for each_dest in self.dest_address:
            self.client.sendto(f"{message}: ".encode(self.DATA_FORMAT), each_dest)
            
        self.operation_idx += 1
        
        # update interface
        self.request_value()

    def query(self):
        return self._eval()

    def _prepare(self, x: str, isAdd: bool):
        if isAdd:
            return f"add:{x}"
        else:
            return f"remove:{x}"

    def _effect(self, message: str):
        if "add" in message:
            x = message.split(":")[1]
            self.A.add(x)
        elif "remove" in message:
            r = message.split(":")[1]
            R = set(x for x in self.A if x[1] == r)
            self.T = self.T.union(R)

    def _eval(self):
        return set(a[1] for a in self.A)

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
                    #TODO: 
                    set_added_elements: Set[Tuple[Tuple[str,int],str]] = set()

                    str_added_elements: str = message.split("@")[0].split(":")[1].strip()

                    sep_added_elements = str_added_elements.split(";")
                    for each_element in sep_added_elements:
                        each_item = each_element.split(",")
                        uid = (each_item[0], int(each_item[1]))
                        x = (uid, each_item[2])
                        set_added_elements.add(x)

                    self._effect(message=str_added_elements)
                
                    # update count label
                    self.request_value()
                elif len(message) > 0 and "remove:" in message:
                    #todo:
                    set_removed_elements: Set[Tuple[Tuple[str,int],str]] = set()

                    str_removed_elements: str = message.split("@")[1].split(":")[1].strip()

                    sep_removed_elements = str_removed_elements.split(";")
                    for each_element in sep_removed_elements:
                        each_item = each_element.split(",")
                        uid = (each_item[0], int(each_item[1]))
                        x = (uid, each_item[2])
                        set_removed_elements.add(x)

                    self._effect(message=str_removed_elements)
                
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