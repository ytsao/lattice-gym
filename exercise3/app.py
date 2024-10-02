import sys
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


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MyApp()
    window.show()
    app.exec()
