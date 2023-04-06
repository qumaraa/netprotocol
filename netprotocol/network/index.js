const net = require('net');
const http = require('http');
const fs = require('fs');
const socketio = require('socket.io');

const port = 2001;

// Create TCP client
const client = new net.Socket();

client.connect(9999, '127.0.0.1', () => {
  console.log('Connected to server');
});

// Create HTTP server
const server = http.createServer((req, res) => {
  fs.readFile('index.html', (err, data) => {
    if (err) {
      res.writeHead(500);
      return res.end('Error loading index.html');
    }
    res.writeHead(200, {'Content-Type': 'text/html'});
    res.end(data);
  });
});

// Start HTTP server
const io = socketio(server);
io.on('connection', (socket) => {
  console.log('A client connected');

  // Listen for incoming messages from the client
  socket.on('message', (message) => {
    console.log(`Received message from client: ${message}`);
  });
});

server.listen(port, () => {
  console.log(`Server listening on port ${port}`);
});

// Handle data received from TCP server
client.on('data', (data) => {
  console.log(`Received data from server: ${data}`);

  // Send data to connected clients via Socket.io
  io.emit('data', data.toString());
});

client.on('close', () => {
  console.log('Connection closed');
});
