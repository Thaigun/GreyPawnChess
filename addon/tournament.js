const prompts = require('prompts');
const greyPawnChess = require('./index');

runTournament();

async function runTournament() {
    const timeControl = await prompts({
        type: 'text',
        name: 'timeControl',
        message: 'Enter time control (e.g. "5+0"): ',
    });
    const timeControlParts = timeControl.timeControl.split('+');
    const initialTime = parseInt(timeControlParts[0]) * 60 * 1000;
    const timeIncrement = parseInt(timeControlParts[1]) * 1000;

    const selectEngines = await prompts({
        type: 'multiselect',
        name: 'engines',
        message: 'Select engines to play against:',
        choices: [
            { title: 'MonteCarlo', value: 'MonteCarlo' },
            { title: 'Random', value: 'Random' },
        ],
        min: 2,
    });

    const engines = selectEngines.engines.map(engine => new greyPawnChess.GreyPawnChess(engine));
    const engineWins = selectEngines.engines.map(() => 0);

    for (let i = 0; i < engines.length; i++) {
        for (let j = i + 1; j < engines.length; j++) {
            await new Promise((resolve) => {
                const engine = engines[i];
                const opponent = engines[j];
                const moves = [];
                engine.setup('w', initialTime, timeIncrement, 'standard');
                opponent.setup('b', initialTime, timeIncrement, 'standard');
                for (const player of [engine, opponent]) {
                    player.updateGameState({
                        type: 'gameState',
                        moves: '',
                        wtime: initialTime,
                        btime: initialTime,
                        winc: timeIncrement,
                        binc: timeIncrement,
                        status: 'started'
                    });
                }
                engine.startGame((move) => {
                    moves.push(move);
                    opponent.updateGameState({
                        type: 'gameState',
                        moves: moves.join(' '),
                        wtime: initialTime,
                        btime: initialTime,
                        winc: timeIncrement,
                        binc: timeIncrement,
                        status: 'started'
                    });
                });
                opponent.startGame((move) => {
                    moves.push(move);
                    engine.updateGameState({
                        type: 'gameState',
                        moves: moves.join(' '),
                        wtime: initialTime,
                        btime: initialTime,
                        winc: timeIncrement,
                        binc: timeIncrement,
                        status: 'started'
                    });
                });
                engine.onGameEnd((result) => {
                    if (result == 'win') {
                        engineWins[i]++;
                    } else if (result = 'draw') {
                        engineWins[i] += 0.5;
                    }
                    resolve();
                });
                opponent.onGameEnd((result) => {
                    if (result == 'win') {
                        engineWins[j]++;
                    } else if (result = 'draw') {
                        engineWins[j] += 0.5;
                    }
                })
            });
        }
    }

    console.log(selectEngines.engines);
    console.log(engineWins);
};