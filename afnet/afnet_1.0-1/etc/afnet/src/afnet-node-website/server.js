/* Imports ************************/
const express = require('express');
const app = express();
const read = require('read-file');
const dotenv = require('dotenv').config();
const exec = require('child_process').exec;
var log = require('debug-logger')('afnet-server.js');

/* Updates *********************/
if(typeof(String.prototype.trim) === "undefined")
{
    String.prototype.trim = function() 
    {
        return String(this).replace(/^\s+|\s+$/g, '');
    };
}

/* Objects ************************/
var afnet = {
  runningServers: [],
  configuredServers: [],
  runningServersHeader: ["Hostname", "Login", "Remote Directory", "Local Directory"],
  configuredServersHeader: ["Hostname", "Login", "Remote Directory", "Something"],
  getRunningServers : function() {
    log.info(process.env.AFHOME);
    var startlist = read.sync(process.env.AFHOME + "/" + process.env.AF_TEXT_START, 'utf8');
    log.debug(startlist);
    for(astart of startlist.split("\n")) {
      var aastart = astart.split(" ");
      astart = astart.trim();
      if(astart.length == 0) continue;
      log.debug(astart);
      var default_values = [process.env.AFNETWORK+"/"+aastart[0]+"/"+aastart[2]];
      default_values[0] = default_values[0].replace(/:/g, "")
      var aaastart = aastart.concat(default_values);
      this.runningServers.push(aaastart);
    }
    log.info(this.configuredServers);
  },
  getConfiguredServers : function() {
    var startlist = read.sync(process.env.AFHOME + "/" + process.env.AF_TEXT_RUNNING, 'utf8');
    for(astart of startlist.split("\n")) {
      astart = astart.trim();
      if(astart.length == 0) continue;
      this.configuredServers.push(astart.split(" "));
    }
  }
};

/* Main: express app ************************/
app.engine('html', require('ejs').renderFile);
app.set('view engine', 'html');
app.use(express.static(__dirname + '/public'));

app.get('/', (req, res) => {
  res.render('index', {
    title: 'Homepage',
  });
});


app.get('/home', (req, res) => {
  afnet.configuredMessage = "";
  afnet.runningMessage = "";
  res.render('home.ejs', {
    title: 'AFNET Home',
    afnet: afnet,
  });
});

app.get('/start', (req, res) => {
  var message = "Starting server index " + req.query.index + " with details server " + req.query.server;
  afnet.configuredMessage = message;
  afnet.runningMessage = message;
  console.log(message);
  res.render('home.ejs', {
    title: 'AFNET Home Start',
    afnet: afnet,
  });
});

app.get('/stop', (req, res) => {
  var message = "Stopping server index " + req.query.index + " with details server " + req.query.server;
  afnet.configuredMessage = message;
  afnet.runningMessage = message;
  console.log(message);
  res.render('home.ejs', {
    title: 'AFNET Home Stop',
    afnet: afnet,
  });
});

/* Main: express server ************************/

const server = app.listen(7000, () => {
  console.log(`Express running → PORT ${server.address().port}`);
});

/* Main: afnet ************************/
afnet.getConfiguredServers();
afnet.getRunningServers();
