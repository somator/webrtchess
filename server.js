const express = require('express')
const app = express()
const server = require('http').Server(app)
const io = require('socket.io')(server)

const port = 3000

app.use(express.static('public'));

app.get('/', (req, res) => {
    res.sendFile(__dirname, '/index.html');
});

function SearchPool() {
    this.pool = {};
    this.timeouts = {};
}

SearchPool.prototype = {
    constructor: SearchPool,

    search: function(user) {
        if (!this.isEmpty()) {
            for (var key in this.pool) {
                if (this.pool[key].available) {
                    this.pool[key].available = false;
                    io.to(user.socketId).emit("peer found", this.pool[key].peerId);
					io.to(this.pool[key].socketId).emit("peer found", user.peerId);
                    delete this.pool[key];
                    clearTimeout(this.timeouts[key]);
                    return;
                }
            }
        }
        else {
            this.wait(user);
        }
    },

    wait: function(user) {
        this.pool[user.peerId] = user;
		this.timeouts[user.peerId] = setTimeout( ()=> {
			delete this.pool[user.peerId];
			this.search(user);
		}, 30000)
    },

    isEmpty: function() {
        for (var i in this.pool) return false;
        return true;
    },
}

var globalPool = new SearchPool();

io.on('connection', (socket) => {
    console.log('client connected');
    socket.join(socket.id);

    socket.on('disconnect', () => {
        console.log('client disconnected');
    });

    socket.on('offer connection', function(user){
		console.log("Received offer", user.peerId);
		globalPool.search(user);
		console.log('Global Pool:\n', globalPool.pool);
	});

});

server.listen(port, () => {
    console.log(`listening on port: ${port}`);
});