const readline = require('readline');
const greypawnchess = require('./index');

let game = new greypawnchess.GreyPawnChess("MonteCarlo");

let rl = readline.createInterface({input: process.stdin, output: process.stdout});
// Get the desired color and time control from the user.
rl.question('Enter your color (w/b): ', (color) => {
    rl.question('Enter time control (e.g. "5+0"): ', (timeControl) => {
        const timeControlParts = timeControl.split('+');
        const initialTime = parseInt(timeControlParts[0]) * 60 * 1000;
        const timeIncrement = parseInt(timeControlParts[1]) * 1000;

        game.setup(color == 'w' ? 'b' : 'w', initialTime, timeIncrement, 'standard');
        game.updateGameState({
            type: 'gameState',
            moves: '',
            wtime: initialTime,
            btime: initialTime,
            winc: timeIncrement,
            binc: timeIncrement,
            status: 'started'
        });
        let moves = [];

        game.startGame((move) => {
            console.log(move);
            moves.push(move);

            rl.question('Enter your move: ', (playerMove) => {
                moves.push(playerMove);

                game.updateGameState({
                        type: 'gameState',
                        moves: moves.join(' '),
                        wtime: initialTime,
                        btime: initialTime,
                        winc: timeIncrement,
                        binc: timeIncrement,
                        status: 'started'
                });
            });
        });

        if (color == 'w') {
            rl.question('Enter your move: ', (playerMove) => {
                moves.push(playerMove);

                game.updateGameState({
                        type: 'gameState',
                        moves: moves.join(' '),
                        wtime: initialTime,
                        btime: initialTime,
                        winc: timeIncrement,
                        binc: timeIncrement,
                        status: 'started'
                });
            });
        }
    });
});