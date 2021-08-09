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