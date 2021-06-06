const express = require('express')
const app = express()
const server = require('http').Server(app)
const io = require('socket.io')(server)

const port = 3000

app.use(express.static('public'));

app.get('/', (req, res) => {
    res.sendFile(__dirname, '/index.html');
});

io.on('connection', (socket) => {
    console.log('client connected');
    socket.on('disconnect', () => {
        console.log('client disconnected');
    });
});

server.listen(port, () => {
    console.log(`listening on port: ${port}`);
});