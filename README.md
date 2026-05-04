# Multiplayer Tanks Game (Linux, C, IPC)

This project is a 2-player terminal-based tank game developed in C, running on Linux.

## Features
- Real-time gameplay in terminal using ncurses
- Two-player interaction via separate processes
- Shared game state using shared memory
- Synchronization using semaphores
- Movement, shooting mechanics, and life system

## Technologies
- C programming language
- Linux system calls
- ncurses
- Inter-Process Communication (IPC):
  - shared memory (shmget, shmat)
  - semaphores (semget, semop)

## How it works
The game runs as two separate processes (one for each player), which communicate through shared memory.  
Semaphores are used to synchronize access to the shared game state and avoid race conditions.

## Compile
```bash
gcc tanks.c -o tanks -lncurses
```

## Run
```bash
./tanks board1 A w s a d f
./tanks board1 B i k j l o
```

## Notes
- Developed as part of an Operating Systems course
- Tested in a Linux (Ubuntu) virtual machine environment
