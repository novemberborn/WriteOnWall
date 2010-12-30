var when = require("promised-io/promise").when;

var config = require("iniparser").parseSync(require("path").join(process.env.HOME, ".anymeta"))[process.argv[2]];
if(!config){
  console.error("Could not determine anymeta config for <%s>", process.argv[2]);
  process.exit(1);
}else{
  require("./anymeta").setup(config);
}

if(!config.outputdir){
  console.error("No output dir specified in .anymeta config!");
  process.exit(1);
}

var tags = require("./tags");
var tagHandler = new tags.TagHandler;
var openFrameworks = new (require("./of").OpenFrameworks);
var webClients = new (require("./web").WebClientManager);

tagHandler.on("tag", function(event, tag){
  console.log("Tag %s[%s]", tag, event);
  when(tags.lookupUser(tag), function(user){
    console.log("Tag %s[%s] --> %s", tag, event, user && user.title || "anonymous");
    webClients.send({ type: "tag", event: event, user: user });
  });
});
tagHandler.on("clear", function(){
  openFrameworks.send("clear");
});
tagHandler.on("like", function(tag){
  when(openFrameworks.capture(), function(capture){
    when(tags.lookupUser(tag), function(user){
      if(user){
        console.log("Capture <%s>[like] --> %s", capture.src, user.title);
        capture.like(user);
      }
    });
  }, function(err){
    console.error("Failed to capture image for tag %s\n%s", tag, err && err.stack);
  });
});
tagHandler.on("made", function(tag){
  when(openFrameworks.capture(), function(capture){
    when(tags.lookupUser(tag), function(user){
      if(user){
        console.log("Capture <%s>[made] --> %s", capture.src, user.title);
        capture.made(user);
      }
    });
  }, function(err){
    console.error("Failed to capture image for tag %s\n%s", tag, err && err.stack);
  });
});

var idle = false;
openFrameworks.on("active", function(){
  idle = false;
  console.log("Active");
  webClients.send({ type: "statechange", state: "active" });
});
openFrameworks.on("idle", function(){
  idle = true;
  console.log("Idle");
  webClients.send({ type: "statechange", state: "idle" });
});
webClients.on("connect", function(client){
  idle && client.send({ type: "statechange", state: "idle" });
});

var fileServer = new (require("node-static").Server)("../web", { cache: 0 });
var httpServer = require("http").Server(function(req, res){
  tagHandler.handle(req).then(
      function(){
        res.writeHead(200);
        res.end();
      },
      function(){
        fileServer.serve(req, res);
      });
});
webClients.listen(httpServer);
httpServer.listen(8080, "0.0.0.0");
openFrameworks.listen(8081, "0.0.0.0");
openFrameworks.watch(config.outputdir);
