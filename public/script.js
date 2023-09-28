const boardElement = document.getElementById('chessboard');
const pawnPromotionModal = document.getElementById('pawnPromotionModal');
const checkmateModal = document.getElementById('checkmateModal');

// Open socket connection to host that serves the page
var socket = io();
socket.on('connect', () => {
    // The Peer object is where we create and receive peer-to-peer connections
    var peer = new Peer();
    // Initiate game
    const game = new Game(peer, boardElement);
});

////////////////////////////////////////////////////////////////

// Color enum
const PlayerColor = {
    White: 'white',
    Black: 'black',
}

// Piece type enum
const PieceType = {
    Pawn: 'pawn',
    Knight: 'knight',
    Bishop: 'bishop',
    Rook: 'rook',
    Queen: 'queen',
    King: 'king',
}

// Pieces in Forsyth-Edwards Notation are represented by chars
// This Json will be used to look up color and piece type
const FenJson = {
    'P': {'color': PlayerColor.White, 'piece': PieceType.Pawn},
    'N': {'color': PlayerColor.White, 'piece': PieceType.Knight},
    'B': {'color': PlayerColor.White, 'piece': PieceType.Bishop},
    'R': {'color': PlayerColor.White, 'piece': PieceType.Rook},
    'Q': {'color': PlayerColor.White, 'piece': PieceType.Queen},
    'K': {'color': PlayerColor.White, 'piece': PieceType.King},
    'p': {'color': PlayerColor.Black, 'piece': PieceType.Pawn},
    'n': {'color': PlayerColor.Black, 'piece': PieceType.Knight},
    'b': {'color': PlayerColor.Black, 'piece': PieceType.Bishop},
    'r': {'color': PlayerColor.Black, 'piece': PieceType.Rook},
    'q': {'color': PlayerColor.Black, 'piece': PieceType.Queen},
    'k': {'color': PlayerColor.Black, 'piece': PieceType.King},
}

// Helper functions

// Return a reversed string
function reverseString(str) {
    return str.split('').reverse().join('');
}

// a -> 1, b -> 2, c -> 3...
function letterToNumber(c) {
    return c.charCodeAt(0) - 96;
}

// Return true if a square is light
function isLightSquare(file, rank) {
    if ((parseInt(rank) + letterToNumber(file)) % 2 == 1) {
        return true;
    } else {
        return false;
    }
}

// cwrapped functions, implementation in chess.c
const set_start_bitboards = Module.cwrap('set_start_bitboards', null);
const find_moves = Module.cwrap('find_moves', 'number', ['string', 'number', 'number']);
const make_move = Module.cwrap('make_move', 'string', ['string', 'string']);
const detect_pawn_promotion = Module.cwrap('detect_pawn_promotion', 'string', []);
const promote_pawn = Module.cwrap('promote_pawn', 'string', ['string', 'number']);
const detect_checkmate = Module.cwrap('detect_checkmate', 'number', ['number']);

// The Game class is responsible handling the game interface and transmitting messages between players
// Game logic is handled by calls to cwrapped functions
class Game {
    constructor(peer, boardElement, fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {
        this.boardElement = boardElement;
        this.fen = fen;
        this.perspective;
        this.selectedSquare;
        this.potentialMoves = [];
        this.outgoingConnection;
        // Every Peer object is assigned a random, unique ID when it's created.
        // When we want to connect to another peer, we'll need to know their peer id.
        peer.on('open', (id) => {
            // Send connection offer to the socket with peerId
            socket.emit('offer connection', {
                peerId: id,
                available: true,
            });
            // When the client receives a peer found message from the host
            socket.on('peer found', (data) => {
                // Determine White or Black from host message
                this.perspective = data.myPlayerColor ? PlayerColor.Black : PlayerColor.White;
                // Listen for incoming messages from peer
                peer.on('connection', (incomingConnection) => {
                    incomingConnection.on('data', (data) => {
                        // Handle incoming data
                        this.handleIncomingMove(data);
                    });
                });
                // Initiate outgoing connection to peer
                this.outgoingConnection = peer.connect(data.opponentPeerId);
                // Set interface and bitboards
                this.annotateSquares();
                this.addPiecesToPawnPromotionModal();
                set_start_bitboards();
                this.fillBoardFromFen();
                // White goes first
                if (this.perspective == PlayerColor.White) {
                    this.listenForMoves();
                }
            });
        });
    }

    // Assign rank and file as id attribute to square elements according to perspective
    annotateSquares(){
        let filesLeftToRight = 'abcdefgh';
        let ranksTopToBottom = '12345678';
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

    // Append piece image as a child of a (square) element
    addPieceToSquare(square, pieceColor, pieceType) {
        let pieceImg = document.createElement('img');
        pieceImg.className = 'piece ' + pieceColor + ' ' + pieceType;
        pieceImg.src = 'piecepics/' + pieceColor + '_' + pieceType + '.png';
        square.appendChild(pieceImg);
    }

    // Append piece images as children of pawn promotion modal
    addPiecesToPawnPromotionModal() {
        this.addPieceToSquare(document.getElementById('promoteToQueen'), this.perspective, PieceType.Queen);
        this.addPieceToSquare(document.getElementById('promoteToRook'), this.perspective, PieceType.Rook);
        this.addPieceToSquare(document.getElementById('promoteToBishop'), this.perspective, PieceType.Bishop);
        this.addPieceToSquare(document.getElementById('promoteToKnight'), this.perspective, PieceType.Knight);
    }

    // Fill the board squares with pieces according to the fen string and the player perspective
    fillBoardFromFen() {
        // Piece placement is the first section of the fen string
        let piecePlacement = this.fen.split(' ')[0];
        // Reverse piece placement if black
        if (this.perspective == PlayerColor.Black) {
            piecePlacement = reverseString(piecePlacement);
        }
        let squares = this.boardElement.children;
        let squareIndex = 0;
        // Iterate through piece placement string
        for (const char of piecePlacement) {
            // Numbers in piece placement string indicate blank spaces
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
            // Letter characters in piece placement string correspond to specific pieces
            } else if (char.match(/[a-z]/i)) {
                let square = squares[squareIndex];
                // remove piece if there is one
                let previous_piece = square.querySelector('.piece')
                if (previous_piece) {
                    square.removeChild(previous_piece);
                }
                squareIndex += 1;
                // look up color and piece type of piece by indexing FenJson and add it to square
                this.addPieceToSquare(square, FenJson[char].color, FenJson[char].piece);
            }
        }
    }

    // Call cwrapped pawn_promotion function once promotion is selected
    listenForPawnPromotion(startSquare, endSquare) {
        pawnPromotionModal.style.display = "block";
        const promotions = pawnPromotionModal.querySelectorAll('.promotion');
        // Add event listeners to each pawn promotion option
        promotions.forEach(promotion => {
            promotion.addEventListener('click', () => {
                let promotionNumber = parseInt(promotion.getAttribute('data-num'));
                promotionNumber = this.perspective == PlayerColor.White ? promotionNumber : promotionNumber + 6;
                // Update the bitboards and store the resulting fen string in this.fen
                this.fen = promote_pawn(endSquare.id, promotionNumber);
                // Update interface using fen string
                this.fillBoardFromFen();
                // Make the pawn promotion modal invisible
                pawnPromotionModal.style.display = "none";
                // Transmit the move info to peer
                this.outgoingConnection.send({
                    'startPos': startSquare.id,
                    'endPos': endSquare.id,
                    'pawnPromotion': promotionNumber
                });
            })
        })
    }

    // Add event listeners to each piece that can be selected by the user
    listenForMoves() {
        const pieces = this.boardElement.querySelectorAll('.' + this.perspective + '.piece');
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

    // Highlight a piece, its potential moves, and listen for potential move selection
    selectSquare(square) {
        var game = this;
        if (this.selectedSquare) {
            this.deselectSquare(this.selectedSquare);
        }
        this.selectedSquare = square;
        square.className = 'square highlighted';
        // cwrapped find_moves will store its result in a pointer
        const movesPtr = find_moves(square.id, 0, 1);
        // read the result
        const moves = this.readMoves(movesPtr);
        const pieceColor = square.querySelector('.piece').classList[1];
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
                game.movePiece(square, moveSquare, pieceColor);
                game.deselectSquare(square);
            });
            this.potentialMoves.push(moveSquare);
        }
    }

    // Remove event listeners and highlighting
    deselectSquare(square) {
        isLightSquare(square.id[0], square.id[1]) ? square.className = 'square light' : square.className = 'square dark';
        for (let potentialMove of this.potentialMoves) {
            potentialMove.removeEventListener('click', potentialMove.ml);
            potentialMove.removeChild(potentialMove.querySelector('.move'));
        }
        this.potentialMoves = [];
        this.selectedSquare = null;
    }

    // Read the moves generated by cwrapped find_moves() and stored in the moves pointer
    // Return them as a JavaScript array
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

    // Handle move selected by the user
    movePiece(startSquare, endSquare, pieceColor) {
        // Update the bitboards and store the resulting fen string in this.fen
        this.fen = make_move(startSquare.id, endSquare.id);
        // Update the interface using the fen string
        this.fillBoardFromFen();
        if (detect_pawn_promotion()) {
            // Prompt the user for pawn promotion selection
            this.listenForPawnPromotion(startSquare, endSquare);
        } else {
            // Transmit the move info to peer
            this.outgoingConnection.send({
                'startPos': startSquare.id,
                'endPos': endSquare.id,
                'pawnPromotion': null
            });
        }
        // If checkmate has occurred
        if (detect_checkmate(pieceColor != PlayerColor.White)) {
            // Make visible the checkmate modal
            checkmateModal.style.display="block";
        }
    }

    // Handle moves transmitted by peer
    handleIncomingMove(data) {
        // Update the fen representation and bitboards by calling cwrapped functions
        this.fen = make_move(data['startPos'], data['endPos']);
        if (data['pawnPromotion']) {
            this.fen = promote_pawn(data['endPos'], data['promotionNumber']);
        }
        // Update the interface using the fen representation
        this.fillBoardFromFen();
        // Determine if I've been checkmated
        if (detect_checkmate(this.perspective == PlayerColor.White)) {
            checkmateModal.style.display="block";
        } else {
            // Otherwise user is free to make a move
            this.listenForMoves();
        }
    }
}