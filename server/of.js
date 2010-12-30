var EventEmitter = require("events").EventEmitter,
    fs = require("promised-io/fs"),
    defer = require("promised-io/promise").defer,
    anymeta = require("./anymeta");

var attempt = (function(){
  var when = require("promised-io/promise").when;
  var rescue = function(){};
  return function(value, resolveCallback){
    return when(value, resolveCallback, rescue);
  };
})();

exports.OpenFrameworks = OpenFrameworks;
function OpenFrameworks(){
  EventEmitter.call(this);
  this._streams = [];
  this._lastCapture = null;
  this.idle = false;
  this.reuseCapture = 10000; // Reuse the capture made in the last 10 seconds
}
OpenFrameworks.prototype = Object.create(EventEmitter.prototype, {
  constructor: {
    value: OpenFrameworks,
    enumerable: false
  }
});

OpenFrameworks.prototype.listen = function(port, hostname){
  require("net").Server(function(stream){
    this._streams.push(stream);
    stream.setEncoding("utf8");
    stream.on("data", function(data){
      if(data.indexOf("active") == 0){
        if(this.idle){
          this.idle = false;
          this.emit("active");
        }
      }else if(data.indexOf("idle") == 0){
        if(!this.idle){
          this.idle = true;
          this.emit("idle");
        }
      }
    }.bind(this));
    stream.on("end", function(){
      stream.end();
      var ix = this._streams.indexOf(stream);
      ix != -1 && this._streams.splice(ix, 1);
    }.bind(this));
  }.bind(this)).listen(port, hostname);
};

OpenFrameworks.prototype.send = function(data){
  this._streams.forEach(function(stream){
    stream.write(data);
  });
};

OpenFrameworks.prototype.watch = function(outputDir){
  require("watch").createMonitor(outputDir, function(monitor){
    console.log("Monitoring <%s> for screens", outputDir);
    monitor.on("created", function(f){
      var dfd = this._captureDeferred;
      dfd && fs.readFile(f).then(dfd.resolve.bind(dfd), dfd.reject.bind(dfd));
    }.bind(this));
  }.bind(this));
};

OpenFrameworks.prototype.capture = function(){
  if(this._lastCapture && Date.now() < this._lastCapture.expires){
    return this._lastCapture;
  }else if(this._capturePromise){
    return this._capturePromise;
  }
  
  this.send("capture");
  this._lastCapture = null;
  this._captureDeferred = defer();
  return this._capturePromise = this._captureDeferred.then(
      function(buffer){
        var capture = new Capture(buffer);
        capture.expires = Date.now() + this.reuseCapture;
        return capture;
      }.bind(this),
      function(err){
        this._captureDeferred = this._capturePromise = null;
      }.bind(this));
};

function Capture(buffer){
  this.buffer = buffer;
  this.expires = 0;
}

Capture.prototype.like = function(user){
  this.save("INTEREST", user);
};

Capture.prototype.made = function(user){
  this.save("AUTHOR", user);
};

Capture.prototype.save = function(predicate, user){
  var createAttachment = this.createAttachment.bind(this);
  var publish = this.publish.bind(this);
  var addAuthor = this.addAuthor.bind(this);
  var addInterest = this.addInterest.bind(this);
  
  attempt(createAttachment(3), function(){
    attempt(predicate == "AUTHOR" && addAuthor(3, user), function(){
      attempt(publish(3), function(){
        predicate == "INTEREST" && addInterest(3, user);
      });
    });
  });
};

Capture.prototype.createAttachment = function(retries){
  if(--retries < 0){ throw new Error("Gave up"); }
  
  return this.attachmentId || anymeta.post("anymeta.attachment.create", {
    data: this.buffer.toString("base64"),
    mime: "image/png",
    title: "Wow! Painting at " + new Date().toTimeString().split(":").slice(0, 2).join(":")
  }).then(
      function(response){
        return this.attachmentId = response.thg_id;
      }.bind(this),
      function(){
        return this.createAttachment(retries);
      }.bind(this));
};

Capture.prototype.publish = function(retries){
  if(--retries < 0){ throw new Error("Gave up"); }
  
  return this.published || anymeta.post("anymeta.thing.update", {
    thing_id: this.attachmentId,
    "data[pubstate]": 1
  }).then(
    function(){
      this.published = true;
    }.bind(this),
    function(){
      return this.publish(retries);
    }.bind(this));
};

Capture.prototype.addAuthor = function(retries, user){
  if(--retries < 0){ throw new Error("Gave up"); }
  
  return anymeta.post("anymeta.edge.add", {
    id: this.attachmentId,
    object: user.rsc_id,
    predicate: "AUTHOR"
  }).then(
      null,
      function(){
        return this.addAuthor(retries, user);
      }.bind(this));
};

Capture.prototype.addInterest = function(retries, user){
  if(--retries < 0){ throw new Error("Gave up"); }
  
  return anymeta.post("anymeta.edge.add", {
    id: user.rsc_id,
    object: this.attachmentId,
    modifier_id: user.rsc_id,
    predicate: "INTEREST"
  }).then(
      null,
      function(){
        return this.addInterest(retries, user);
      }.bind(this));
};
