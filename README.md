# GreyPawnChess
Lichess chess bot.

The basic engine functions like finding the available moves are done. The client can connect to Lichess and receive and accept challenges. It knows what happens on the board and when it's its turn to move. It ponders moves in a separate thread and sends the move to the server when it feels it's ready.

However, at the moment it's not very good at playing chess. It uses a Monte-Carlo Tree Search for decision making and with how slow the engine is (it's quite fast, but slow compared to for example Stockfish with finding legal moves) that does not yield very good playing. However, it's a good start!

The engine is implemented as a native C++ addon for Node.js. Connection with the Lichess server is handled by the Node.js client and the moves are decided by the engine.

## Structure
This repository contains three sub-parts:
* addon folder contains the chess engine as a node addon.
  * napi folder contains the node-addon-api middleman between the engine code and Node.
  * engine folder contains the engine as a C++ implementation.
* lichess-bot folder uses the C++ addon and connects to the Lichess API to play some chess.

## Requirements
You need:
* npm, Node.js and node-gyp
  * `npm install -g node-gyp`
* CMake
* node-gyp requires Python
* C++ build tools to be used by node-gyp

## Usage
To run the tests:
* `cd addon`
* `npm test`
The tests might run for a while. The engine is finding possible moves to the depth of several moves for various positions.

To run the engine, follow these steps:
* Create a Lichess bot account. Instructions can be found [here](https://lichess.org/api#operation/apiBotOnline) (Dec 21st 2021)
* Create file `lichess-bot/lichess.config`. Add the username of your bot (which you registered in the first step) and the API key. Follow same format as shown in `lichess-bot/lichess.config.example`.
* From the project root: `cd addon && npm install`
* From the project root: `cd lichess-bot && npm install`
* In lichess-bot folder: `npm start`
Prepare to fight with C++ build-chain and Python.