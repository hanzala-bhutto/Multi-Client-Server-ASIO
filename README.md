# Multi-Client-Server-ASIO

## Table of Contents
- [Introduction](#introduction)
- [Tech Stack](#tech-stack)
- [Features](#features)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Installation](#installation)
  - [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgments](#acknowledgments)

## Introduction

Multi-Client-Server-ASIO is an asynchronous, multi-client server-client framework built using **Boost.Asio** in C++. It allows for scalable communication between a server and multiple clients, efficiently handling simultaneous connections with non-blocking I/O operations. This project demonstrates modern C++ socket programming with clean and modular code.

## Tech Stack

- **C++17**: Modern C++ features for clean and efficient code.
- **Boost.Asio**: A library for asynchronous input/output operations and networking.
- **Visual Studio**: IDE for development and solution management.

## Features

- **Multi-client support**: Efficiently manages multiple simultaneous client connections.
- **Asynchronous communication**: Non-blocking I/O for high performance and responsiveness.
- **Modular architecture**: Separate client, server, and common utility code.
- **Cross-platform compatibility**: Should work on any platform supporting Boost.Asio and C++17.

## Project Structure
```bash
Multi-Client-Server-ASIO/
├── Client/                 # Client-side implementation
│   ├── main.cpp            # Entry point for the client
│   ├── Client.cpp          # Core client logic
│   └── Client.hpp          # Client header file
├── Server/                 # Server-side implementation
│   ├── main.cpp            # Entry point for the server
│   ├── Server.cpp          # Core server logic
│   └── Server.hpp          # Server header file
├── Common/                 # Shared utilities and definitions
│   └── Utilities.hpp       # Helper functions and common definitions
├── .gitattributes          # Git configuration file
├── .gitignore              # Ignored files and directories
├── LICENSE                 # License information
├── README.md               # Documentation
└── Client-Server-App.sln   # Visual Studio solution file
```

## Getting Started

### Prerequisites

Before you begin, ensure you have the following installed:
- A compatible C++ compiler with **C++17** support.
- [Boost C++ Libraries](https://www.boost.org/), especially **Boost.Asio** for networking.
- **Visual Studio** (or any other C++ IDE that supports Boost).

### Installation

To set up the project locally:

- Clone the repository:
   ```bash
   git clone https://github.com/hanzala-bhutto/Multi-Client-Server-ASIO.git

- Open the `Client-Server-App.sln` file in Visual Studio (or your preferred IDE).
- Build the solution to compile the server and client binaries.

### Running the Application

- Start the server application from the `Server` directory.
- Launch the client application(s) from the `Client` directory.
- Observe communication between the server and multiple clients.

### Usage

Once the project is built:

- **Start the server**:
   - Navigate to the `Server/` directory and launch the server application. 
   - The server will listen for incoming client connections and handle communication with them.

- **Launch the client(s)**:
   - Navigate to the `Client/` directory.
   - Launch one or more client applications. Each client will connect to the server and send requests.

- **Monitor interaction**:
   - Observe the communication between the server and the clients. The server should process requests and respond to each connected client.

- **Simulate multiple clients**:
   - To test scalability, you can simulate multiple clients by running several instances of the client application. This allows you to test how the server handles multiple simultaneous connections and requests.

## Contributing

Contributions are welcome! Feel free to fork the repository and submit pull requests.

## License

This project is released under the MIT License. See `LICENSE` for details.

## Acknowledgments

- Thanks to the creators of Boost.Asio for their powerful networking library.
- Inspiration from other open-source ASIO-based projects.
