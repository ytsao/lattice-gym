# Lattice Theory for Parallel Programming @University of Luxembourg
Exercises for the course Lattice Theory for Parallel Programming @University of Luxembourg


## Conflict-Free Replicated Data Types (1-CRDT)

* `1-CRDT/exercise`: contains the exercises (`.py`files).

### Prerequesities
```bash 
git clone https://github.com/ptal/lattice-theory-parallel-programming-uni.lu.git
cd lattice-theory-parallel-programming-uni.lu/1-CRDT/exercise 
conda env create -f environment.yml
```
or
```bash
git clone https://github.com/ptal/lattice-theory-parallel-programming-uni.lu.git
cd lattice-theory-parallel-programming-uni.lu/1-CRDT/exercise 
python3 -m venv <your-env-name>
source <your-env-name>/bin/activate
pip install PyQt6
```

### How to use?

Launch server:
```bash
python3 server.py
```

Launch 2 peers:
```bash 
python3 app.py
python3 app.py
```

**PS: After launching 2 peers, the server can be terminated.**


### Exercise Requirements
