var greypawnchess = require('bindings')('greypawnchess');

// Initializes a new game with the given parameters
let match = new greypawnchess.GreyPawnChess();

//match.setup("w", 180, 3, "standard");

//server.on('data', (d) => {
//    // Parse the move data and then call the engine, also update the remaining time for each player.
//    
//});

match.startGame(function(move) {
    // Send move to the server.
    console.log('move was made:');
    console.log(move);
}); // starts calculating the best moves and calling the callback

console.log("started game");

//console.log(match.updateGameState(/*d.body.moves, d.body.wtime, d.body.btime, d.body.status*/));


let match2 = new greypawnchess.GreyPawnChess();
match2.startGame(function(move) {
    // Send move to the server.
    console.log('move was made:');
    console.log(move);
});

console.log("started another game");

setTimeout(() =>  {
    console.log("delayed log");
    match.stopGame();
    match2.stopGame();
}, 5000);