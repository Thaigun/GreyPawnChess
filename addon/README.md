# Creating a new type of strategy:
* Implement it in a directory in `engine/src/NewStrategyName`.
* The new strategy must implement `class GreyPawnChess`.
* Add the folder in `binding.gyp` to be built.
* Add it as an option in `napi/chessAddon.cpp` constructor of `GreyPawnChessAddon`.
* Now it's usable from Node.js with `new greypawnchess.GreyPawnChess('newStrategyName');`