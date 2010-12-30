var io = require("socket.io"),
    EventEmitter = require("events").EventEmitter;

exports.WebClientManager = WebClientManager;
function WebClientManager(){
  EventEmitter.call(this);
  this._clients = [];
};
WebClientManager.prototype = Object.create(EventEmitter.prototype, {
  constructor: {
    value: WebClientManager,
    enumerable: false
  }
});

WebClientManager.prototype.listen = function(httpServer){
  var socket = io.listen(httpServer, { transports: ["websocket"] });
  socket.on("connection", function(client){
    this._clients.push(client);
    client.on("disconnect", function(){
      var ix = this._clients.indexOf(client);
      ix != -1 && this._clients.splice(ix, 1);
    }.bind(this));
    this.emit("connect", client);
  }.bind(this));
};

WebClientManager.prototype.send = function(msg){
  this._clients.forEach(function(client){
    client.send(msg);
  });
};
