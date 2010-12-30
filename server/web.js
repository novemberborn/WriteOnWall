var io = require("socket.io");

exports.WebClientManager = WebClientManager;
function WebClientManager(){
  this._clients = [];
};

WebClientManager.prototype.listen = function(httpServer){
  var socket = io.listen(httpServer, { transports: ["websocket"] });
  socket.on("connection", function(client){
    this._clients.push(client);
    client.on("disconnect", function(){
      var ix = this._clients.indexOf(client);
      ix != -1 && this._clients.splice(ix, 1);
    }.bind(this));
  }.bind(this));
};

WebClientManager.prototype.send = function(msg){
  this._clients.forEach(function(client){
    client.send(msg);
  });
};
