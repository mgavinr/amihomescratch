/* Imports ************************/
const express = require('express');
const app = express();
const read = require('read-file');
const fs = require('fs');
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
  configuredMessage : "",
  runningMessage : "",
  stoppedMessage : "",
  runningfile : process.env.AFHOME + "/" + process.env.AF_TEXT_RUNNING,
  startfile : process.env.AFHOME + "/" + process.env.AF_TEXT_START,
  stopfile : process.env.AFHOME + "/" + process.env.AF_TEXT_STOP,
  runningServers: [],
  startServers: [],
  stoppedServers: [],
  runningServersHeader: ["Hostname", "Login", "Remote Directory", "Local Directory"],
  startServersHeader:   ["Hostname", "Login", "Remote Directory", "Local Directory"],
  stoppedServersHeader: ["Hostname", "Login", "Remote Directory", "Local Directory"],
  getLocalDirectory : function(file_entry_list) {
    var dir = process.env.AFNETWORK+"/"+file_entry_list[0]+"/"+file_entry_list[2];
    dir = dir.replace(/:/g, "")
    return dir;
  },
  getServers : function(filename) {
    var file_contents = read.sync(filename, 'utf8');
    var servers = []
    for(file_entry of file_contents.split("\n")) {
      file_entry = file_entry.trim();
      if(file_entry.length == 0) continue;
      var file_entry_list = file_entry.split(" ");
      var new_file_entry_list = file_entry_list.concat([this.getLocalDirectory(file_entry_list)]);
      servers.push(new_file_entry_list);
    }
    return servers;
  },
  updateServersAsync : function(filename, value) {
    fs.appendFile(filename, value, (err) => {
      if (err) console.log(err);
      console.log("Added=" + value + " to filename=" + filename);
    });
  },
  updateServers : function(filename, value) {
    try {
      fs.appendFileSync(filename, value);
      console.log("Added=" + value + " to filename=" + filename);
    } catch(err) {
      console.log(err);
    }
  },
  getConfiguredServers : function() {
    runningfile = process.env.AFHOME + "/" + process.env.AF_TEXT_RUNNING,
    startfile = process.env.AFHOME + "/" + process.env.AF_TEXT_START,
    stopfile = process.env.AFHOME + "/" + process.env.AF_TEXT_STOP,
    this.runningServers = this.getServers(this.runningfile);
    this.startServers = this.getServers(this.startfile);
    this.stoppedServers = this.getServers(this.stopfile);
  }
};

/* Main: express app ************************/
app.engine('html', require('ejs').renderFile);
app.set('view engine', 'html');
app.use(express.static(__dirname + '/public'));

app.get('/', (req, res) => {
  afnet.getConfiguredServers();
  res.render('home.ejs', {
    title: 'AFNET Home',
    afnet: afnet,
  });
});


app.get('/home', (req, res) => {
  afnet.getConfiguredServers();
  res.render('home.ejs', {
    title: 'AFNET Home',
    afnet: afnet,
  });
});

app.get('/start', (req, res) => {
  var message = "Starting " + req.query.server + "("+ req.query.index + ").";
  afnet.configuredMessage = "";
  afnet.runningMessage = "";
  afnet.stoppedMessage = "";

  if (req.query.indexname === "stopped") {
    afnet.stoppedMessage = message;
    var data = afnet.stoppedServers[req.query.index].slice(0,3).join(' ')+'\n';
    afnet.updateServers(afnet.startfile, data);
    // TODO remove from the stop file!
    console.log(message);
  } else {
    afnet.configuredMessage = message;
    var data = afnet.startServers[req.query.index].slice(0,3).join(' ')+'\n';
    afnet.updateServers(afnet.startfile, data);
    // TODO remove from the stop file!
    console.log(message);
  }

  res.render('run.ejs', {
    title: 'AFNET Home Start',
    afnet: afnet,
  });
});

app.get('/stop', (req, res) => {
  var message = "Stopping " + req.query.server + "("+ req.query.index + ").";
  afnet.configuredMessage = "";
  afnet.runningMessage = "";
  afnet.stoppedMessage = "";

  if (req.query.indexname === "running") {
    afnet.runningMessage = message;
    var data = afnet.stoppedServers[req.query.index].slice(0,3).join(' ')+'\n';
    afnet.updateServers(afnet.stopfile, data);
    console.log(message);
  } else {
    afnet.configuredMessage = message;
    var data = afnet.startServers[req.query.index].slice(0,3).join(' ')+'\n';
    afnet.updateServers(afnet.stopfile, data);
    console.log(message);
  }

  res.render('run.ejs', {
    title: 'AFNET Home Stop',
    afnet: afnet,
  });
});

/* Main: express server ************************/

const server = app.listen(7000, () => {
  console.log(`Express running → PORT ${server.address().port}`);
});
