var url = require("url"),
    defer = require("promised-io/promise").defer,
    EventEmitter = require("events").EventEmitter,
    anymeta = require("./anymeta");

exports.TagHandler = TagHandler;
function TagHandler(){
  EventEmitter.call(this);
}
TagHandler.prototype = Object.create(EventEmitter.prototype, {
  constructor: {
    value: TagHandler,
    enumerable: false
  }
});

TagHandler.prototype.handle = function(req){
  var pathname = url.parse(req.url).pathname;
  var dfd = defer();
  if(pathname.indexOf("/tag") == 0){
    if(req.method == "POST"){
      var tag = "";
      var event = pathname.split(/\//).pop();
      req.on("data", function(chunk){ tag += chunk; });
      req.on("end", function(){
        this.emit("tag", event, tag);
        this.emit(event, tag);
        dfd.resolve(tag);
      });
    }else{
      dfd.resolve();
    }
  }else{
    dfd.reject();
  }
  return dfd.promise;
};

var lookupCache = {};
exports.lookupUser = function(tag){
  return lookupCache[tag] || anymeta.get("identity.identify", { type: "rfid", raw: "urn:rfid:" + tag }).then(function(user){
    return lookupCache[tag] = user;
  }, function(){
    return null;
  });
};
