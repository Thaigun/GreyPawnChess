# GreyPawnChess
Lichess chess bot.

NOTE: Heavily WIP, expect more extensive instructions during 2021.

## Structure
This repository contains three sub-parts:
* addon folder contains the chess engine as a node addon.
  * napi folder contains the node-addon-api middleman between the engine code and Node.
  * engine folder contains the engine as a C++ implementation. Or will contain. It's not a chess engine yet, really.
* lichess-bot folder uses the native addon and connects to the Lichess API to play some chess.

## Requirements
Some of the requirements may not be needed to run parts of the program, but you need 
* npm and Node.js
  * npm install -g node-gyp
* CMake
* I think node-gyp requires Python
* Perhaps some other C++ build tools to be used by node-gyp?

## Usage
To be updated.
