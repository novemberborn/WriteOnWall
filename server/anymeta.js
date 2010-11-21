var request = require("promised-io/http-client").request,
    toQueryString = require("promised-io/querystring").toQueryString;

var oaa = JSON.parse(require("fs").readFileSync("../web/oaa.json"));
var oauth = new (require("oauth").OAuth)("", "", oaa.consumer.key, oaa.consumer.secret, "1.0", null, "HMAC-SHA1");
var makeRequest = function(httpMethod, apiMethod, params){
  params = params || {};
  params.method = apiMethod;
  params.format = "json";
  
  var url = "http://www.mediamatic.net/services/rest/?" + toQueryString(params);
  var signed = oauth.signUrl(url, oaa.token.key, oaa.token.secret, httpMethod);
  var query = signed.split("?").slice(1).join("?");
  
  var requestObj = {
    method: httpMethod,
    hostname: "www.mediamatic.net",
    pathInfo: "/services/rest/"
  };
  if(httpMethod == "GET" || httpMethod == "DELETE"){
    requestObj.queryString = query;
  }else{
    requestObj.body = [query];
    requestObj.headers = {
      host: "www.mediamatic.net",
      "Content-Type": "application/x-www-form-urlencoded",
      // Set Content-Length to prevent chunking, which Anymeta doesn't like
      "Content-Length": query.length
    };
  }
  return request(requestObj).then(
      function(response){
        return response.body.join("").then(function(body){
          try{
            return JSON.parse(body);
          }catch(e){
            console.error("Error parsing JSON response");
            console.error(body);
            throw body;
          }
        });
      },
      function(err){
        console.error("Error in getting protected resource:");
        console.error(err);
        throw err;
      });
};

exports.get = makeRequest.bind(exports, "GET");
exports.put = makeRequest.bind(exports, "PUT");
exports.post = makeRequest.bind(exports, "POST");
exports.delete = makeRequest.bind(exports, "DELETE");
exports.del = makeRequest.bind(exports, "DELETE");

//exports.get("anymeta.user.info").then(function(response){
//  console.log(response.title);
//});

// exports.post("anymeta.attachment.create", { data: "", mime: "image/png" }).then(function(response){
//   console.dir(response);
// });
