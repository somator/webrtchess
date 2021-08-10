var socket = io();
socket.on('connect', () => {
    console.log(socket.id)
    var peer = new Peer();
    
    peer.on('open', function(id) {
        console.log('My peer ID is: ' + id);
        console.log('My socket ID is: ' + socket.id);

        socket.emit('offer connection', {
            peerId: id,
            socketId: socket.id,
            available: true,
        });

        socket.on('peer found', peerId => {
            console.log("Im connecting to other user: " + peerId);
            var conn = peer.connect(peerId);
            conn.on('open', function() {
                console.log('connection open');
            });
        });
    });
});

////////////////////////////////////////////////////////////////

let chessBoard = document.getElementById('chessboard');

const files = 'abcdefgh';
const ranks = '12345678';

const PlayerColor = {
    White: 'white',
    Black: 'black',
}

function reverseString(str) {
    return str.split('').reverse().join('');
}

// a -> 1, b -> 2, c -> 2...
function letterToNumber(c) {
    return c.charCodeAt(0) - 96;
}

function isLightSquare(file, rank) {
    if ((parseInt(rank) + letterToNumber(file)) % 2 == 1) {
        return true;
    } else {
        return false;
    }
}

function appendSquare(board, squareColor, fileString, rankString) {
    let square = document.createElement('div');
    square.className = 'square ' + squareColor;
    square.id = fileString + rankString;
    board.appendChild(square);
}

function generateSquares(board, perspective){
    let filesLeftToRight = files;
    let ranksTopToBottom = ranks;
    if (perspective == PlayerColor.White) {
        ranksTopToBottom = reverseString(ranksTopToBottom);
    } else {
        filesLeftToRight = reverseString(filesLeftToRight);
    }
    for (let row = 0; row < 8; row++) {
        let rank = ranksTopToBottom[row];
        for (let col = 0; col < 8; col++) {
            let file = filesLeftToRight[col];
            if (isLightSquare(file, rank)) {
                appendSquare(board, 'light', file, rank);
            } else {
                appendSquare(board, 'dark', file, rank);
            }
        }
    }
}

generateSquares(chessBoard, PlayerColor.White);