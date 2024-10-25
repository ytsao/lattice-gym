# lattice-gym

Welcome to lattice-gym!
In this repository, we're providing serveral conflict-free replication data type (CRDT) examples in simple socket application.

## Prerequisties
```
git clone git@github.com:ytsao/lattice-gym.git
cd lattice-gym
conda env -f environment.yml
```

## Supporting types of CRDT

**state-based CRDT:**
1. G-counter
2. D-counter
3. PN-counter
4. 2P-set
5. OR-set

**operation-based CRDT:**
1. PN-counter
2. OR-set

### Example
In this example environment, we have 1 server to send the all clients information to each client and we have 2 clients to receive and send their current state.

**Terminal 1 - turn-on server:**
```
conda activate lattice-gym
python server.py
Enter the number of clients in the counter: 2
```

**Terminal 2 - turn-on client 1:**
```
cd lattice-gym/state_based_CRDT
conda activate lattice-gym
python PN-counter.py
```

**Terminal 3 - turn-on client 2:**
```
cd lattice-gym/state_based_CRDT
conda activate lattice-gym
python PN-counter.py
```

After that, you can use the application to synchronize the increment and decrement counter with another client.