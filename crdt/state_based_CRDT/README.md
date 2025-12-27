# Conflict-Free Replicated Data Types (1-CRDT)

* `1-CRDT/exercise`: contains the exercises (`.py`files).

## Prerequesities
```bash 
git clone https://github.com/ptal/lattice-theory-parallel-programming-uni.lu.git
cd lattice-theory-parallel-programming-uni.lu/1-CRDT/exercise 
conda env create -f environment.yml
conda activate lattice_gym
```
or
```bash
git clone https://github.com/ptal/lattice-theory-parallel-programming-uni.lu.git
cd lattice-theory-parallel-programming-uni.lu/1-CRDT/exercise 
python3 -m venv <your-env-name>
source <your-env-name>/bin/activate
pip install PyQt6
```

## How to use?

Launch server:
```bash
python3 server.py
```

Launch 2 peers:
```bash 
python3 client.py # client 1
python3 client.py # client 2
```

**PS: After launching 2 peers, the server can be terminated.**


## Exercise G-Counter CRDT

1. Implement the G-Counter CRDT as seen in class. You can use a simple gossip protocol between the clients triggered every 10 seconds through the socket interface.
2. Is the counter of the clients eventually consistent? Test your implementation with different scenario.
