var when = require("promised-io/promise").when;

var anymetaConfig = require("iniparser").parseSync(require("path").join(process.env.HOME, ".anymeta"))[process.argv[2]];
if(!anymetaConfig){
  console.error("Could not determine anymeta config for <%s>", process.argv[2]);
  process.exit(1);
}else{
  require("./anymeta").setup(anymetaConfig);
}

var outputDir = process.argv[3];
if(!outputDir){
  console.error("No output dir specified!");
  process.exit(1);
}

var tags = require("./tags");
var tagHandler = new tags.TagHandler;
var openFrameworks = new (require("./of").OpenFrameworks);
var webClients = new (require("./web").WebClientManager);

tagHandler.on("tag", function(event, tag){
  when(tags.lookupUser(tag), function(user){
    webClients.send({ type: "tag", event: event, user: user });
  });
});
tagHandler.on("clear", function(){
  openFrameworks.send("clear");
});
tagHandler.on("like", function(tag){
  when(openFrameworks.capture(), function(capture){
    when(tags.lookupUser(tag), function(user){
      user && capture.like(user);
    });
  });
});
tagHandler.on("made", function(tag){
  when(openFrameworks.capture(), function(capture){
    when(tags.lookupUser(tag), function(user){
      user && capture.made(user);
    });
  });
});

openFrameworks.on("active", function(){
  webClients.send({ type: "statechange", state: "active" });
});
openFrameworks.on("idle", function(){
  webClients.send({ type: "statechange", state: "idle" });
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
openFrameworks.watch(outputDir);
