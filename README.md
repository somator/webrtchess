# WebRTChess

WebRTChess is a peer-to-peer in-browser chess application made with C compiled to WebAssembly, utilizing the WebRTC framework.

# Installation
## Prerequisites
Download and install [Node.js](https://nodejs.org/en/download)  

Download, install, and activate [Emscripten](https://emscripten.org/docs/getting_started/downloads.html)

Inside the webrtchess folder, run
```
npm install
```
to install Node dependencies.

## Compilation
Compile chess.c with emcc, exporting the necessary functions
```
emcc -s EXPORTED_FUNCTIONS=_set_start_bitboards,_find_moves,_make_move,_detect_pawn_promotion,_promote_pawn,_detect_checkmate -s EXPORTED_RUNTIME_METHODS='["cwrap", "UTF8ToString", "getValue", "setValue"]' chess.c
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
In your browser, go to localhost:3000. Open another tab/window to localhost:3000. Once 2 clients have entered the host's client pool, a peer to peer connection will be established and the game will start.