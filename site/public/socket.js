socket = io();
POST();
function POST(){
    setTimeout(function(){
        if(socket.connected && document.readyState == "complete")
            loaded();
        else
            POST();
    },100);
}

function openPort(path, freq) {
    var port = {
        name: path,
        rate: freq
    }
    socket.emit("openPort", port);
}

function closePort(name) {
    var port = {
        name: name
    }
    socket.emit("closePort", port);
}

var app = angular.module("Sensor-Monitor", []);
app.controller("side-view", function ($scope) {
    $scope.ports = [];
    $scope.listeners = [];

    socket.on("ports", function (ports) {
        $scope.$apply(function () {
            $scope.ports = ports;
        });
    })
    socket.on("listeners", function (listeners) {
        $scope.$apply(function () {
            $scope.listeners = listeners;
        });
    })
});

app.controller("telemetry", function ($scope) {
    $scope.report = {};
    socket.on("report", function (report) {
        $scope.$apply(function () {
            $scope.report = report;
        });
    })
});