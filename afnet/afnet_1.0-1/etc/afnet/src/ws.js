var http = require('http');
var utils = require('./utils');
var url = require('url');

http.createServer(function (req, res) {
  var par = url.parse(req.url, true).query;
  res.writeHead(200, {'Content-Type': 'text/html'});
  res.write("<br>The date and time are currently: " + utils.myDateTime());
  res.write("<br>");
  res.write(req.url);
  res.end('<br>Hello World!');
}).listen(8080);
