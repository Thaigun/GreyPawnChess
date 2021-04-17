// Node library requires
const fs = require('fs');
const https = require('https');
const path = require('path');

// Third party requires
const axios = require('axios');
const ndjson = require('ndjson');

// Chess engine require
const greypawnchess = require('bindings')('greypawnchess');

// Setup and config
const baseUrl = 'https://lichess.org/api/';
const configFile = fs.readFileSync(path.join(__dirname, 'lichess.config'));
const config = JSON.parse(configFile);
const authString = 'Bearer ' + config.apiKey;
const userName = config.userName;

const gameEndStatuses = [
    'aborted', 'mate', 'resign', 'stalemate', 'timeout',
    'draw', 'outoftime', 'cheat', 'nostart', 'unknownfinish',
    'variantend'
];

const axiosInstance = axios.create({
    baseURL: baseUrl,
    headers: {
        'Authorization': authString
    }
});

https.get(baseUrl + 'stream/event', {
    headers: {
        'Authorization': authString
    }}, 
    (res) => {
        console.log('Listening to Lichess event stream.');
        res.pipe(ndjson.parse({strict: false})).on('data', (data) => {
            console.log(data);
            handleMessage(data);
        });
        // TODO: Handle other messages, like error, end etc.
    }
);

let ongoingGames = [];

function handleMessage(msg) {
    switch (msg.type) {
        case 'challenge':
            if (ongoingGames.length > 0 || msg.challenge.variant.key !== 'standard') {
                declineChallenge(msg);
            } else {
                acceptChallenge(msg);
            }
            break;
        case 'gameStart':
            if (ongoingGames.length > 0) {
                abortGame(msg);
            } else {
                startGame(msg);
            }
            break;
        case 'gameFinish':
            console.log('Game finished.');
            stopGame(msg);
            break;
    }
}

async function declineChallenge(msg) {
    try {
        let declineResponse = await axiosInstance.post('/challenge/' + msg.challenge.id + '/decline');
    } catch (err) {
        console.error(err);
    }
}

async function acceptChallenge(msg) {
    try {
        let acceptResponse = await axiosInstance.post('/challenge/' + msg.challenge.id + '/accept');
        if (acceptResponse.status == 200) {
            console.log('Starting a game.');
        }
    } catch (err) {
        console.error(err);
    }
}

async function abortGame(msg) {
    try {
        let abortResponse = await axiosInstance.post('/bot/game/' + msg.game.id + '/abort');
    } catch (err) {
        console.error(err);
    }
}

function startGame(msg) {
    let newGame = new greypawnchess.GreyPawnChess();
    let storedGameObj = {
        id: msg.game.id,
        setupDone: false,
        game: newGame
    };
    ongoingGames.push(storedGameObj);

    // Get a stream of game events.
    https.get(baseUrl + 'bot/game/stream/' + msg.game.id, {
        headers: {
            'Authorization': authString
        }}, (res) => {
            let dataStream = res.pipe(ndjson.parse({strict: false}));
            storedGameObj.connection = res;
            dataStream.on('data', (data) => {
                console.log(data);
                handleGameUpdate(data);
            });
            dataStream.on('close', () => {
                console.log('Game closed');
            });
        }
    );
}

function handleGameUpdate(msg) {
    let gameIdx = ongoingGames.findIndex(g => g.id === msg.id);
    if (gameIdx == -1)
        return;

    let ongoingGame = ongoingGames[gameIdx];
    switch (msg.type) {
        case 'gameFull':
            if (!ongoingGame.setupDone) {
                let myColor = 'w';
                if (msg.black.name === userName) {
                    myColor = 'b';
                }
                // Get time control, increments and variant.
                // TODO: Setup properly
                ongoingGame.game.setup(myColor, msg.clock.initial, msg.clock.increment, msg.variant.key);
                ongoingGame.setupDone = true;
                ongoingGame.game.startGame((move) => {
                    makeMove(ongoingGame.id, move);
                });
                ongoingGame.game.updateGameState(msg.state);
            }
            break;
        case 'gameState':
            ongoingGame.game.updateGameState(msg);
            break;
    }
}

async function makeMove(gameId, move) {
    try {
        for (let i = 0; i < 10; i++) {
            let moveResponse = await axiosInstance.post('/bot/game/' + gameId + '/move/' + move);
            if (moveResponse == 200)
                break;
        }        
    } catch (err) {
        console.error(err);
    }
}

function stopGame(msg) {
    let gameIdx = ongoingGames.findIndex(g => g.id === msg.game.id);
    if (gameIdx == -1)
        return;

    let ongoingGame = ongoingGames[gameIdx];
    ongoingGame.connection.destroy();
    ongoingGame.game.stopGame();
    ongoingGames.splice(gameIdx, 1);
}