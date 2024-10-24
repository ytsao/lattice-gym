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
                
        self.add_button.clicked.connect(partial(self.update, isAdd=True))
        self.remove_button.clicked.connect(partial(self.update, isAdd=False)) 
        self.lookup_button.clicked.connect(self.lookup)


    def request_value(self):
        # user interface function
        self.state_label.setText(f"Current State: {self._value()}\n A: {self.A}\n T: {self.T}")

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
            uid = (self.node_id, self.operation_idx)
            x = (uid, add_value)
            self.A.add(x)
            self.operation_idx += 1

    def _remove(self):
        # CRDT implementation
        remove_value: str = self.remove_text.text().strip()
        if remove_value != "Enter the element to remove":
            R = set(x for x in self.A if x[1] == remove_value)
            self.A = self.A - R
            self.T = self.T.union(R)
    
    def _lookup(self, lookup_value: str):
        # CRDT implementation
        value = self._value()
        return lookup_value in value

    def _merge(self, other_A: set, other_T: set):
        # called asynchronously
        self.A = (self.A-other_T).union(other_A-self.T)
        self.T = self.T.union(other_T)

    def _value(self):
        # CRDT implementation -> user interface : value
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
                elif len(message) > 0 and "OR-set A:" in message and "OR-set T:" in message:
                    set_added_elements: Set[Tuple[Tuple[str,int],str]] = set()
                    set_removed_elements: Set[Tuple[Tuple[str,int],str]] = set()

                    str_added_elements: str = message.split("@")[0].split(":")[1].strip()
                    str_removed_elements: str = message.split("@")[1].split(":")[1].strip()

                    if str_added_elements != "":
                        sep_added_elements = str_added_elements.split(";")
                        for each_element in sep_added_elements:
                            each_item = each_element.split(",")
                            uid = (int(each_item[0]), int(each_item[1]))
                            x = (uid, each_item[2])
                            set_added_elements.add(x)
                    
                    if str_removed_elements != "":
                        sep_removed_elements = str_removed_elements.split(";")
                        for each_element in sep_removed_elements:
                            each_item = each_element.split(",")
                            uid = (int(each_item[0]), int(each_item[1]))
                            x = (uid, each_item[2])
                            set_removed_elements.add(x)

                    self._merge(other_A=set_added_elements, other_T=set_removed_elements)
                
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
            str_added_removed_elements: str = "OR-set A:"
            for a in self.A:
                str_added_removed_elements += f"{a[0][0]},{a[0][1]},{a[1]};"
            if str_added_removed_elements[-1] == ";":
                str_added_removed_elements = str_added_removed_elements[:-1]
            str_added_removed_elements += "@OR-set T:"
            for t in self.T:
                str_added_removed_elements += f"{t[0][0]},{t[0][1]},{t[1]};"
            if str_added_removed_elements[-1] == ";":
                str_added_removed_elements = str_added_removed_elements[:-1]

            # send to all others 
            for each_dest_address in self.dest_address:
                if len(str_added_removed_elements) != 0:
                    self.client.sendto(f"{str_added_removed_elements}".encode(self.DATA_FORMAT), each_dest_address)
                

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MyApp()
    t1 = threading.Thread(target=window.start, daemon=True)
    t2 = threading.Thread(target=window.receive, daemon=True)
    t1.start()
    t2.start()
    window.show()
    app.exec()
