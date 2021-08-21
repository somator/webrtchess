const boardElement = document.getElementById('chessboard');

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
                const game = new Game(boardElement, myPlayerColor);
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

// a -> 1, b -> 2, c -> 3...
function letterToNumber(c) {
    return c.charCodeAt(0) - 96;
}

function anToBitboard(an) {
    const file = an[0]
    const rank = an[1];
    if (rank < '5') {
        const lower = 1 << ((8 * (rank.charCodeAt(0) - 49)) + (104 - file.charCodeAt(0)));
        return new Bitboard(u32(0), u32(lower));
    } else {
        const upper = 1 << ((8 * (rank.charCodeAt(0) - 53)) + (104 - file.charCodeAt(0)));
        return new Bitboard(u32(upper), u32(0));
    }
}

function isLightSquare(file, rank) {
    if ((parseInt(rank) + letterToNumber(file)) % 2 == 1) {
        return true;
    } else {
        return false;
    }
}

function getPieceElem(square) {
    return square.querySelector('.piece');
}

function getPiece(square) {
    const pieceElem = getPieceElem(square);
    if (pieceElem) {
        const piece = {
            color: pieceElem.classList[1],
            type: pieceElem.classList[2],
        }
        return piece;
    } else {
        return null;
    }
}

class Game {
    constructor(boardElement, perspective, fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {
        this.boardElement = boardElement;
        this.perspective = perspective;
        this.fen = fen;
        this.annotateSquares();
        this.fillBoardFromFen();
        this.listenForMoves();
    }

    // Assign rank and file to square elements according to perspective
    // To be used when the squares have already been added to the board
    annotateSquares(){
        let filesLeftToRight = files;
        let ranksTopToBottom = ranks;
        if (this.perspective == PlayerColor.White) {
            ranksTopToBottom = reverseString(ranksTopToBottom);
        } else {
            filesLeftToRight = reverseString(filesLeftToRight);
        }
        for (let i = 0; i < 64; i++) {
            let fileIndex = i % 8;
            let rankIndex = Math.trunc(i/8);
            let fileAndRank = filesLeftToRight[fileIndex] + ranksTopToBottom[rankIndex];
            this.boardElement.children[i].setAttribute('id', fileAndRank);
        }
    }

    // Append piece image as a child of a square element
    addPieceToSquare(square, pieceName) {
        let pieceImg = document.createElement('img');
        pieceImg.className = 'piece ' + pieceName;
        pieceImg.src = 'piecepics/' + pieceName + '.png';
        square.appendChild(pieceImg);
    }

    // Fill the board squares with pieces according to the fen string and the player perspective
    fillBoardFromFen() {
        let piecePlacement = this.fen.split(' ')[0];
        if (this.perspective == PlayerColor.Black) {
            piecePlacement = reverseString(piecePlacement);
        }
        let squares = this.boardElement.children;
        let squareIndex = 0;
        for (const char of piecePlacement) {
            if (!isNaN(char)) {
                squareIndex += parseInt(char);
            } else if (char.match(/[a-z]/i)) {
                let square = squares[squareIndex];
                squareIndex += 1;
                switch(char) {
                    case 'P':
                        this.addPieceToSquare(square, 'white_pawn');
                        break;
                    case 'N':
                        this.addPieceToSquare(square, 'white_knight');
                        break;
                    case 'B':
                        this.addPieceToSquare(square, 'white_bishop');
                        break;
                    case 'R':
                        this.addPieceToSquare(square, 'white_rook');
                        break;
                    case 'Q':
                        this.addPieceToSquare(square, 'white_queen');
                        break;
                    case 'K':
                        this.addPieceToSquare(square, 'white_king');
                        break;
                    case 'p':
                        this.addPieceToSquare(square, 'black_pawn');
                        break;
                    case 'n':
                        this.addPieceToSquare(square, 'black_knight');
                        break;
                    case 'b':
                        this.addPieceToSquare(square, 'black_bishop');
                        break;
                    case 'r':
                        this.addPieceToSquare(square, 'black_rook');
                        break;
                    case 'q':
                        this.addPieceToSquare(square, 'black_queen');
                        break;
                    case 'k':
                        this.addPieceToSquare(square, 'black_king');
                        break;
                    default:
                        alert("An error occurred while parsing FEN.");
                        break;
                }
            }
        }
    }

    listenForMoves() {
        const pieces = this.boardElement.querySelectorAll('.piece')
        pieces.forEach(piece => {
            piece.addEventListener('click', () => {
                const square = piece.parentElement;
                if (square.className != 'square highlighted') {
                    square.className = 'square highlighted';
                    this.findMoves(square);
                } else {
                    isLightSquare(square.id[0], square.id[1]) ? square.className = 'square light' : square.className = 'square dark';
                }
            })
        })
    }

    findMoves(square) {
        // to do
        return;
    }
}

/* appendSquare(board, squareColor, fileString, rankString) {
    let square = document.createElement('div');
    square.className = 'square ' + squareColor;
    square.id = fileString + rankString;
    board.appendChild(square);
}

// Generate square elements for board and assign them rank/file/color according to the player's perspective
generateSquares(board, perspective){
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
} */