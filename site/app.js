var express = require("express");
var app = express();
var http = require("http").createServer(app);
var io = require("socket.io")(http);
var path = require("path");
var dispatcher = require("./dispatcher");

var port = 80;

app.use(express.static(__dirname + "/public/"));

app.get("/", function (req, res) {
  res.sendFile(path.join(__dirname + "/public/index.html"));
});

dispatcher.event.on("update", function (scope, msg) {
  io.emit(scope, msg);
})

dispatcher.event.on("report", function(report){
  io.emit("report", report);
})

io.on("connect", function (socket) {
  socket.on("request", function (msg) {
    switch (msg) {
      case "ports":
        var portList = dispatcher.portList;
        socket.send("ports", portList);
        break;

      case "listeners":
        socket.send("listeners", dispatcher.listeners);
        break;

      default:
        socket.send("error", "Invalid request");
        break;
    }
  })

  socket.on("openPort", function(port){
    dispatcher.addListener(port.name, port.rate);
  })

  socket.on("closePort", function(port){
    dispatcher.removeListener(port.name);
  })
});

http.listen(port, function () {
  console.log("listening on " + port);
});
