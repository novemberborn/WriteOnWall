dojo.provide("wow.Anymeta");

dojo.require("dojox.io.OAuth");

dojo.declare("wow.Anymeta", null, {
  constructor: function(endpoint, oaa){
    this.endpoint = endpoint;
    this.oaa = oaa;
  },
  
  call: function(httpMethod, xhrArgs){
    dojox.io.OAuth.sign(httpMethod, xhrArgs, this.oaa);
    xhrArgs.url = "/anymeta";
    return dojo.xhr(httpMethod, xhrArgs);
  },
  
  get: function(method, params){
    params = params || {};
    params.method = method;
    params.format = "json";
    return this.call("GET", { url: this.endpoint, content: dojo.clone(params) }).then(dojo.hitch(this, function(response){
      if(response == "false"){
        throw new Error("No valid response");
      }
      return JSON.parse(response);
    }));
  }
});