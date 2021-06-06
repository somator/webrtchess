const express = require('express')
const app = express()
const server = require('http').Server(app)

const port = 3000

app.get('/', (req, res) => {
    res.send('<h1>Hello world</h1>');
});

server.listen(port, () => {
    console.log(`listening on port: ${port}`);
});