const greypawnchess = require('bindings')('greypawnchess');
const https = require('https');
const ndjson = require('ndjson');
const axios = require('axios');

let baseUrl = 'https://lichess.org/api/';

const axiosInstance = axios.create({
    baseURL: baseUrl,
    headers: {
        'Authorization': 'Bearer LCD3mh8QjtU815Jt'
    }
});

https.get(baseUrl + 'stream/event', {
    headers: {
        'Authorization': 'Bearer LCD3mh8QjtU815Jt'
    }}, 
    (res) => {
        res.pipe(ndjson.parse({strict: false})).on('data', (data) => {
            console.log(data);
            handleMessage(data);
        });
    }
);

let ongoingGames = [];

function handleMessage(msg) {
    switch (msg.type) {
        case 'challenge':
            if (ongoingGames.length > 0) {
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
    ongoingGames.push({
        id: msg.game.id,
        setupDone: false,
        game: newGame
    });
    // TODO: Close the connection when the game ends.
    https.get(baseUrl + 'bot/game/stream/' + msg.game.id, {
        headers: {
            'Authorization': 'Bearer xxx'
        }}, 
        (res) => {
            res.pipe(ndjson.parse({strict: false})).on('data', (data) => {
                console.log(data);
                handleGameUpdate(data);
            });
        }
    );
}

function handleGameUpdate(msg) {
    // TODO: Implement
    let ongoingGame = ongoingGames.find(g => g.id === msg.id);
    switch (msg.type) {
        case 'gameFull':
            if (!ongoingGame.setupDone) {
                // TODO: compare player id's to tell if we are b/w
                // Get time control, increments and variant.
                ongoingGame.game.setup()
                ongoingGame.setupDone = true;
                // TODO: Remember to stop on game ending status.
                ongoingGame.game.startGame((move) => {
                    makeMove(ongoingGame.id, move);
                });
            }
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