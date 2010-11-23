var http = require("http"),
    url = require("url"),
    static = require("node-static"),
    fs = require("promised-io/fs"),
    when = require("promised-io/promise").when,
    defer = require("promised-io/promise").defer,
    request = require("promised-io/http-client").request,
    io = require("socket.io"),
    applescript = require("applescript"),
    watch = require("watch"),
    anymeta = require("./anymeta");

var outputDir = process.argv[2];
if(!outputDir){
  console.error("No output dir specified!");
  process.exit(1);
}else{
  console.log("Monitoring <%s> for screens", outputDir);
}

var fileServer = new static.Server("../web", { cache: 0 });
var httpServer = http.Server(function(req, res){
  var info = url.parse(req.url, true);
  
  // console.log(req.url);
  if(info.pathname.indexOf("/tag") == 0){
    var tag = "";
    var id = info.pathname.split(/\//).pop();
    req.on("data", function(chunk){ tag += chunk; });
    req.on("end", function(){
      var msg = JSON.stringify({ type: "tag", state: req.method == "POST" ? "add" : "remove", data: tag, id: id });
      console.log(msg);
      clients.forEach(function(client){
        client.send(msg);
      });
      res.writeHead(200);
      res.end();
      
      if(id == "like" || id == "made"){
        actOnScreen(tag, id);
      }else if(id == "clear"){
        clearScreen(id);
      }
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

var outputQueue = [];

function saveScreen(){
  if(outputQueue.length > 0){
    return outputQueue[0].promise;
  }
  
  var dfd = defer();
  applescript.execString('activate application "opencvExampleDebug"\ntell application "System Events" to key code 36', function(){});
  outputQueue.push(dfd);
  return dfd.promise;
}

watch.createMonitor(outputDir, function(monitor){
  monitor.on("created", function(f){
    if(!outputQueue.length){ return; }
    
    var dfd = outputQueue.shift();
    fs.readFile(f).then(dfd.resolve.bind(dfd));
  });
});

var tags = {};
function identifyUser(tag){
  return tags[tag] || anymeta.get("identity.identify", { type: "rfid", raw: "urn:rfid:" + tag }).then(function(user){
    return tags[tag] = user;
  });
}

var lastScreen;
var USE_LAST_FOR = 10000;
function actOnScreen(tag, id){
  var user = identifyUser(tag);
  var screen;
  if(lastScreen && Date.now() - lastScreen.time < USE_LAST_FOR){
    screen = lastScreen.screen;
  }else{
    screen = saveScreen();
    screen.then(function(img){
      lastScreen = {
        screen: img,
        time: Date.now()
      };
    });
  }
  when(screen, function(img){
    when(user, function(user){
        console.log("Identified user for tag %s", tag);
        // console.dir(user);
        
        var attachment = img.attachment;
        if(!attachment){
          attachment = img.attachment = anymeta.post("anymeta.attachment.create", {
            data: img.toString("base64"),
            mime: "image/png"
          }).then(function(response){
            console.log("Uploaded image");
            // console.dir(response);
            return response;
          });
        }
        
        when(attachment, function(response){
          console.log("Adding edge…");
          var params = {};
          if(id == "like"){
            params.id = user.rsc_id;
            params.object = response.thg_id;
            params.modifier_id = user.rsc_id;
            params.predicate = "INTEREST";
          }else{
            params.id = response.thg_id;
            params.object = user.rsc_id;
            params.predicate = "AUTHOR";
          }
          anymeta.post("anymeta.edge.add", params).then(function(response){
            var finishedAdding = true;
            if(id == "made"){
              var params = {
                id: response.thg_id,
                object: user.rsc_id,
                predicate: "ACTOR"
              };
              finishedAdding = anymeta.post("anymeta.edge.add", params);
            }
            
            when(finishedAdding, function(){
              console.log("Added edges");
              console.dir(response);
              anymeta.post("anymeta.thing.update", { thing_id: response.thg_id, "data[pubstate]": 1 }).then(function(){
                console.log("Published!");
              });
            });
          });
        });
      },
      function(err){
        console.error("Couldn't find user for tag %s", tag);
      });
  });
}

function clearScreen(){
  applescript.execString('activate application "opencvExampleDebug"\ntell application "System Events" to key code 49', function(){});
}

var idle = false;
var net = require("net");
net.Server(function(stream){
  stream.setEncoding("utf8");
  stream.on("data", function(data){
    var msg = { type: "statechange" };
    if(data.indexOf("active") == 0){
      msg.state = "active";
      idle = false;
    }
    if(data.indexOf("idle") == 0){
      msg.state = "idle";
      idle = Date.now();
    }
    msg = JSON.stringify(msg);
    clients.forEach(function(client){
      client.send(msg);
    });
    setTimeout(function(){
      if(idle && Date.now() - idle >= 5 * 60 * 1000){
        var msg = JSON.stringify({ type: "requestClear" });
        clients.forEach(function(client){
          client.send(msg);
        });
      }
    }, 5 * 60 * 1000);
  });
  stream.on("end", function(){
    stream.end();
  });
}).listen(8081, "0.0.0.0");