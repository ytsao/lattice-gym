import sys
from typing import List
from PyQt6.QtWidgets import QApplication, QWidget, QLabel, QPushButton, QVBoxLayout


class MyApp(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Lattice Gym")
        self.resize(300, 300)  # width, height

        layout = QVBoxLayout()
        self.setLayout(layout)

        # widgets
        self.count_label = QLabel()
        self.count_label.setText("Hello, just for testing")
        self.push_botton = QPushButton()
        self.push_botton.setText("Click me : )")

        layout.addWidget(self.count_label)
        layout.addWidget(self.push_botton)

        self.count: List[int, int] = [0, 0]
        self.total_count: int = 0

        self.push_botton.clicked.connect(self.counter)

    def counter(self):
        self.count[0] += 1
        self.count_label.setText(f"Count from this client: {self.count[0]} \n Total count: {self.total_count}")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MyApp()
    window.show()
    app.exec()
