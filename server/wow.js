var http = require("http"),
    url = require("url"),
    static = require("node-static"),
    when = require("promised-io/promise").when,
    request = require("promised-io/http-client").request,
    io = require("socket.io");

var fileServer = new static.Server("../web", { cache: 0 });
var httpServer = http.Server(function(req, res){
  var info = url.parse(req.url, true);
  
  if(info.pathname == "/tag"){
    var tag = "";
    req.on("data", function(chunk){ tag += chunk; });
    req.on("end", function(){
      var msg = JSON.stringify({ type: "tag", state: req.method == "POST" ? "add" : "remove", data: tag });
      console.log(msg);
      clients.forEach(function(client){
        client.send(msg);
      });
      res.writeHead(200);
      res.end();
    });
  }else if(info.pathname == "/anymeta"){
    // TODO: POST requests
    var proxy = request({
      url: "http://www.mediamatic.net/services/rest/" + info.search
    });
    when(proxy, function(result){
      res.writeHead(result.status, result.headers);
      result.body.join("").then(function(body){
        res.end(body);
      });
    });
  }else{
    fileServer.serve(req, res);
  }
});

var socket = io.listen(httpServer, { transports: ["websocket"] });
var clients = [];
socket.on("connection", function(client){
  clients.push(client);
  client.on("disconnect", function(){
    var ix = clients.indexOf(client);
    ix != -1 && clients.splice(ix, 1);
  });
});
httpServer.listen(8080, "0.0.0.0");
