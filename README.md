# Networked Turn-Based Reversi Game

This project implements a two-player Reversi game using a client-server architecture. The server handles game logic, board state, and communication, while the client provides the user interface for players to interact with the game.

## Features

- **Multiplayer Support**: Two players connect to a central server to play Reversi on an 8x8 board.
- **Game Rules**: Players take turns placing pieces, flipping the opponent's pieces in straight lines. The game ends when the board is full, no valid moves are possible, or one player loses all their pieces. The player with the most pieces wins.
- **Threaded Server**: Each game is handled in a separate thread, allowing multiple games to run simultaneously.
- **TCP Communication**: Reliable data transmission using TCP sockets.
- **ANSI Escape Codes**: Console-based client with colored text for better user experience.
