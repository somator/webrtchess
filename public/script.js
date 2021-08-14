let chessBoard = document.getElementById('chessboard');

// Open socket connection to host that serves the page
var socket = io();
socket.on('connect', () => {
    console.log(socket.id)
    // The Peer object is where we create and receive peer-to-peer connections
    var peer = new Peer();
    
    // Every Peer object is assigned a random, unique ID when it's created.
    // When we want to connect to another peer, we'll need to know their peer id.
    peer.on('open', function(id) {
        console.log('My peer ID is: ' + id);
        console.log('My socket ID is: ' + socket.id);

        // Send connection offer to the socket with peerId
        socket.emit('offer connection', {
            peerId: id,
            available: true,
        });

        // When the client receives a peer found message from the host
        socket.on('peer found', (data) => {
            console.log("Im connecting to other user: " + data.opponentPeerId);
            // Open the peer-to-peer connection using the opponent's peer ID
            var conn = peer.connect(data.opponentPeerId);
            conn.on('open', function() {
                console.log('connection open');
                const myPlayerColor = data.myPlayerColor ? PlayerColor.Black : PlayerColor.White;
                annotateSquares(chessBoard, myPlayerColor);
            });
        });
    });
});

////////////////////////////////////////////////////////////////

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

// Generate square elements for board and assign them rank/file/color according to the player's perspective
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

// Assign rank and file to square elements according to perspective
// To be used when the squares have already been added to the board
function annotateSquares(board, perspective){
    let filesLeftToRight = files;
    let ranksTopToBottom = ranks;
    if (perspective == PlayerColor.White) {
        ranksTopToBottom = reverseString(ranksTopToBottom);
    } else {
        filesLeftToRight = reverseString(filesLeftToRight);
    }
    for (let i = 0; i < 64; i++) {
        let fileIndex = i % 8;
        let rankIndex = Math.trunc(i/8);
        let fileAndRank = filesLeftToRight[fileIndex] + ranksTopToBottom[rankIndex];
        board.children[i].setAttribute('id', fileAndRank);
    }
}

// generateSquares(chessBoard, PlayerColor.White);
// annotateSquares(chessBoard, PlayerColor.White);