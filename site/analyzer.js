var events = require("events");
var eventEmitter = new events.EventEmitter();
var date = require('date-and-time');

exports.event = eventEmitter;

exports.feed = function (data) {
    if (data[0] == 1 && data[1] == 0 && data[data.length - 1] == 4) {
        var report = new Report();
        var BME280 = new Sensor("BME280");
        BME280.add(new Reading("Temperature", data.readFloatLE(2), "C"));
        BME280.add(new Reading("Pressure", data.readFloatLE(6) / 100, "hPa"));
        BME280.add(new Reading("Samples", data.readInt16LE(10)));
        var MCP9808 = new Sensor("MCP9808");
        MCP9808.add(new Reading("Temperature", data.readFloatLE(12), "C"));
        MCP9808.add(new Reading("Samples", data.readInt16LE(16)));
        report.add(BME280);
        report.add(MCP9808);
        eventEmitter.emit("report", report);
    }
}

exports.once = function() {
    var report = new Report();
    var BME280 = new Sensor("BME280");
    BME280.add(new Reading("Temperature", 2.3, "C"));
    BME280.add(new Reading("Pressure", 2.4, ""));
    BME280.add(new Reading("Samples", 5));
    var MCP9808 = new Sensor("MCP9808");
    MCP9808.add(new Reading("Temperature", 32.2, "C"));
    MCP9808.add(new Reading("Samples", 1));
    report.add(BME280);
    report.add(MCP9808);
    eventEmitter.emit("report", report);
}

function Reading(name,reading, unit = ""){
    this.name = name;
    this.reading = reading;
    this.unit = unit;
}

function Sensor(name){
    this.name = name;
    this.readings = [];
}
Sensor.prototype.add = function(reading){
    this.readings.push(reading);
}

function Report(){
    this.sensors = [];
    this.date = new Date();
}
Report.prototype.add = function(sensor){
    this.sensors.push(sensor);
}