![image](https://github.com/user-attachments/assets/99a26bd7-87f2-4a16-b8cc-fab494bacdfb)

Translated text:\
UNISINOS\
COURSE: Computer Science\
SUBJECT: Software Engineering - Fundamentals Prof. Flávio Oliveira\
SCHOOL Polytechnic\
Case 3: Shelter management system for homeless/refugee people.\
The management of shelters for homeless or refugee people involves several challenges, such as the control and efficient\
distribution of resources, monitoring the well-being of the sheltered people and the coordination of the shelter's daily activities.\
Often, this management is based on manual and error-prone processes, making it difficult to make quick and informed decisions.\
The lack of a clear and real-time view of needs and availability can lead to underutilization or lack of essential resources,\
directly affecting the quality of the service provided to the sheltered people. In addition, documentation and individualized\
monitoring of residents are complex and fundamental tasks to ensure that each person receives the necessary support.\
Our task is to develop an efficient automated management solution to guarantee the organization, safety and well-being of residents,\
facilitating communication between the different actors involved and promoting a more human and welcoming environment.


# Shelter Management System

A system to manage shelters for homeless and refugee populations, focusing on efficiency, safety, and organization.

## Overview

This project aims to automate some common shelter operations and tasks:

- Tracking and distributing resources  
- Managing resident records and support needs  
- Coordinating staff and daily activities  
- Centralizing communication

This was done in C, using Sqlite3 for persistent database and UI was done using raylib/raygui

## Build & Run

Requires `make`, a C compiler (`clang` or `gcc`), and necessary libraries.

### Build Targets

```bash
make debug       # Debug build with sanitizers  
make release     # Optimized release build  
make clean       # Remove build artifacts
```

To run, double click the .exe or .out, or run on the terminal with

```bash
./main.exe
./tests.exe
# or for linux
./main.out
./tests.out
```

## Run Targets

```bash
make run         # Build and run the application (debug)  
make test        # Run test suite  
make app         # Clean, build release, and run application
```

Binaries are created in the root directory, object files are placed in `out/`.

## Documentation

This project uses Doxygen for source code documentation.

To generate:

```bash
doxygen Doxyfile
```

Documentation output will be saved as defined in the `Doxyfile` settings, default in a `doxyfile` directory.
