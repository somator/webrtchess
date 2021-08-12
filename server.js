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
                    // randomBoolean has a 50/50 chance of being true or false
                    // this is used to determine which player is white and which is black
                    var randomBoolean = Math.random() < 0.5;
                    io.to(user.socketId).emit("peer found", {
                        opponentPeerId: this.pool[key].peerId, 
                        myPlayerColor: randomBoolean
                    });
					io.to(this.pool[key].socketId).emit("peer found", {
                        opponentPeerId: user.peerId,
                        myPlayerColor: !randomBoolean
                    });
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
        user.socketId = socket.id;
		globalPool.search(user);
		console.log('Global Pool:\n', globalPool.pool);
	});

});

server.listen(port, () => {
    console.log(`listening on port: ${port}`);
});