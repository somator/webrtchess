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

function isLightSquare(file, rank) {
    if ((parseInt(rank) + letterToNumber(file)) % 2 == 1) {
        return true;
    } else {
        return false;
    }
}

// cwrapped functions
const set_start_bitboards = Module.cwrap('set_start_bitboards', null);
const find_moves = Module.cwrap('find_moves', 'number', ['string', 'number', 'number']);
const make_move = Module.cwrap('make_move', 'string', ['string', 'string']);
const detect_pawn_promotion = Module.cwrap('detect_pawn_promotion', 'string', []);
const detect_checkmate = Module.cwrap('detect_checkmate', 'number', ['number']);

class Game {
    constructor(boardElement, perspective, fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {
        this.boardElement = boardElement;
        this.perspective = perspective;
        this.fen = fen;
        this.selectedSquare;
        this.potentialMoves = [];
        this.annotateSquares();
        set_start_bitboards();
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
    addPieceToSquare(square, pieceColor, pieceType) {
        let pieceImg = document.createElement('img');
        pieceImg.className = 'piece ' + pieceColor + ' ' + pieceType;
        pieceImg.src = 'piecepics/' + pieceColor + '_' + pieceType + '.png';
        square.appendChild(pieceImg);
    }

    // Fill the board squares with pieces according to the fen string and the player perspective
    fillBoardFromFen() {
        // Interface
        let piecePlacement = this.fen.split(' ')[0];
        if (this.perspective == PlayerColor.Black) {
            piecePlacement = reverseString(piecePlacement);
        }
        let squares = this.boardElement.children;
        let squareIndex = 0;
        for (const char of piecePlacement) {
            if (!isNaN(char)) {
                for (let i = 0; i < parseInt(char); i++) {
                    let square = squares[squareIndex];
                    // remove piece if there is one
                    let previous_piece = square.querySelector('.piece')
                    if (previous_piece) {
                        square.removeChild(previous_piece);
                    }
                    squareIndex += 1;
                }
                // squareIndex += parseInt(char);
            } else if (char.match(/[a-z]/i)) {
                let square = squares[squareIndex];
                // remove piece if there is one
                let previous_piece = square.querySelector('.piece')
                if (previous_piece) {
                    square.removeChild(previous_piece);
                }
                squareIndex += 1;
                switch(char) {
                    case 'P':
                        this.addPieceToSquare(square, 'white', 'pawn');
                        break;
                    case 'N':
                        this.addPieceToSquare(square, 'white', 'knight');
                        break;
                    case 'B':
                        this.addPieceToSquare(square, 'white', 'bishop');
                        break;
                    case 'R':
                        this.addPieceToSquare(square, 'white', 'rook');
                        break;
                    case 'Q':
                        this.addPieceToSquare(square, 'white', 'queen');
                        break;
                    case 'K':
                        this.addPieceToSquare(square, 'white', 'king');
                        break;
                    case 'p':
                        this.addPieceToSquare(square, 'black', 'pawn');
                        break;
                    case 'n':
                        this.addPieceToSquare(square, 'black', 'knight');
                        break;
                    case 'b':
                        this.addPieceToSquare(square, 'black', 'bishop');
                        break;
                    case 'r':
                        this.addPieceToSquare(square, 'black', 'rook');
                        break;
                    case 'q':
                        this.addPieceToSquare(square, 'black', 'queen');
                        break;
                    case 'k':
                        this.addPieceToSquare(square, 'black', 'king');
                        break;
                    default:
                        alert("An error occurred while parsing FEN.");
                        break;
                }
            }
        }
    }

    listenForMoves() {
        const game = this;
        const pieces = this.boardElement.querySelectorAll('.piece');
        pieces.forEach(piece => {
            piece.addEventListener('click', () => {
                const square = piece.parentElement;
                if (square.className != 'square highlighted') {
                    this.selectSquare(square);
                } else {
                    this.deselectSquare(square);
                }
            })
        })
    }

    selectSquare(square) {
        var game = this;
        if (this.selectedSquare) {
            this.deselectSquare(this.selectedSquare);
        }
        this.selectedSquare = square;
        square.className = 'square highlighted';
        const movesPtr = find_moves(square.id, 0, 1);
        const moves = this.readMoves(movesPtr);
        for (let an of moves) {
            const moveSquare = document.getElementById(an);
            const potentialMoveSignifier = document.createElement('div');
            if (moveSquare.querySelector('.piece')) {
                potentialMoveSignifier.className = 'move capture-circle'
            } else {
                potentialMoveSignifier.className = 'move move-circle';
            }
            moveSquare.appendChild(potentialMoveSignifier);
            moveSquare.addEventListener('click', moveSquare.ml = function moveListener() {
                game.movePiece(square, moveSquare);
                game.deselectSquare(square);
            });
            this.potentialMoves.push(moveSquare);
        }
    }

    deselectSquare(square) {
        isLightSquare(square.id[0], square.id[1]) ? square.className = 'square light' : square.className = 'square dark';
        for (let potentialMove of this.potentialMoves) {
            potentialMove.removeEventListener('click', potentialMove.ml);
            potentialMove.removeChild(potentialMove.querySelector('.move'));
        }
        this.potentialMoves = [];
        this.selectedSquare = null;
    }

    readMoves(movesPtr) {
        const moves = [];
        for (let i = 0; i < 21; i++) {
            const str = Module.UTF8ToString(movesPtr + 2*i, 2);
            if (str) {
                moves.push(str);
            }
            else { break; }
        }
        return moves;
    }

    movePiece(startSquare, endSquare) {
        this.fen = make_move(startSquare.id, endSquare.id);
        this.fillBoardFromFen();
        this.listenForMoves();
    }
}