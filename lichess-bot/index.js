import fs from 'fs';
import https from 'https';
import path from 'path';
import axios from 'axios';
import ndjson from 'ndjson';
import greypawnchess from 'grey-pawn-chess';

// Setup and config
const baseUrl = 'https://lichess.org/api/';
const configFile = fs.readFileSync('lichess.config');
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
    let newGame = new greypawnchess.GreyPawnChess("MonteCarlo");
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
                handleGameUpdate(data, storedGameObj);
            });
            dataStream.on('close', () => {
                console.log('Game closed');
            });
        }
    );
}

function handleGameUpdate(msg, ongoingGame) {
    switch (msg.type) {
        case 'gameFull':
            if (!ongoingGame.setupDone) {
                let myColor = 'w';
                if (msg.black.name === userName) {
                    myColor = 'b';
                }
                // Get time control, increments and variant.
                ongoingGame.game.setup(myColor, msg.clock.initial, msg.clock.increment, msg.variant.key);
                ongoingGame.setupDone = true;
                ongoingGame.game.updateGameState(msg.state);
                ongoingGame.game.startGame((move) => {
                    makeMove(ongoingGame.id, move);
                });
            }
            break;
        case 'gameState':
            console.log('Calling game state update from Node.');
            ongoingGame.game.updateGameState(msg);
            break;
    }
}

async function makeMove(gameId, move) {
    try {
        console.log('Making move ' + move);
        let moveResponse = await axiosInstance.post('/bot/game/' + gameId + '/move/' + move);
    } catch (err) {
        console.log('Error when sending move ' + move);
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