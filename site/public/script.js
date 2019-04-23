function loaded(){
    document.getElementById("loading-status").classList.remove("loading");
    document.getElementById("loading").onclick = function(){
        removeLoadscreen();
    }
}

function removeLoadscreen(){
    document.getElementById("loading").classList.remove("loading");
    setTimeout(function(){
        document.getElementById("loading").style.display = "none";
    },500)
}

function port(elem){
    var port = elem.innerHTML.trim();
    console.log(port);
    openPort(port, 19200);
}

function listener(elem){
    var port = elem.innerHTML.substring(0, elem.innerHTML.indexOf("(")).trim();
    console.log(port);
    closePort(port);
}