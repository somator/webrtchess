# WebRTChess

WebRTChess is a peer-to-peer in-browser chess application made with C compiled to WebAssembly, utilizing the WebRTC framework.

# Installation
## Prerequisites
Download and install [Node.js](https://nodejs.org/en/download)  

Install [Express](https://expressjs.com/)            
```
npm install express
```
Install [PeerServer for PeerJS](https://peerjs.com/)
```
npm install peer -g
```
Download, install, and activate [Emscripten](https://emscripten.org/docs/getting_started/downloads.html)

## Compilation
Compile chess.c with emcc, exporting the necessary functions
```
emcc -s EXPORTED_FUNCTIONS=_set_start_bitboards,_find_moves,_make_move,_detect_pawn_promotion,_detect_checkmate -s EXPORTED_RUNTIME_METHODS='["cwrap", "UTF8ToString", "getValue", "setValue"]' chess.c
```

# Usage
## Running locally
Run server.js
```
node server.js
```
Run the PeerServer
```
peerjs --port 3001
```
In your browser, go to localhost:3000.