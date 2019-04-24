var serialport = require("serialport");
var analyzer = require("./analyzer");

var events = require("events");
var eventEmitter = new events.EventEmitter();

var listeners = []; // list of all ports with listeners attatched
var portList = [];

exports.listeners = listeners;
exports.event = eventEmitter;
exports.portList = portList;

exports.addListener = function (path, rate) {
    for (var s = 0; s < listeners.length; s++) {
        if (listeners[s].path === path) {
            return;
        }
    }
    var port = new serialport(path, { baudRate: rate });
    listeners.push(port);
    eventEmitter.emit("update", "listeners", listeners);

    var dataTemp = Buffer.alloc(0);
    port.on("data", function (buffer) {
        if(dataTemp == null){
            if(buffer[0] == 1 && buffer[buffer.length-1] != 4)
                dataTemp = Buffer.concat([dataTemp, buffer], dataTemp.length + buffer.length);
        }
        else{
            if(buffer[buffer.length-1] == 4){
                analyzer.feed(Buffer.concat([dataTemp, buffer], dataTemp.length + buffer.length));
                dataTemp = null;
            }
        }
    });

    port.on("error", function (err) {
        eventEmitter.emit("error", err, port);
    });

    port.on("close", function () {
        eventEmitter.emit("close", port);

    });
}

analyzer.event.on("report", function (report) {
    eventEmitter.emit("report", report);
})

exports.once = function () {
    analyzer.once();
}

exports.removeListener = function (path) {
    for (var s = 0; s < listeners.length; s++) {
        if (listeners[s].path == path) {
            listeners[s].close();
            listeners[s] = listeners[listeners.length - 1];
            listeners.length--;
            eventEmitter.emit("update", "listeners", listeners);
        }
    }

}

var listnersOldCount = listeners.length;
var clientUpdate = setInterval(function () {
    // send portList
    serialport.list(function (err, ports) {
        portList = ports;
        eventEmitter.emit("update", "ports", portList);
    })
    // see for inactive listeners
    for (var s = 0; s < listeners.length; s++) {
        if (!listeners[s].isOpen && !listeners[s].opening) {
            listeners[s] = listeners[listeners.length - 1];
            listeners.length--;
        }
    }
    // send listeners
    eventEmitter.emit("update", "listeners", listeners);
}, 5000);