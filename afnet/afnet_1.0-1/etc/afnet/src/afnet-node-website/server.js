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
  stoppedfile : process.env.AFHOME + "/" + process.env.AF_TEXT_STOP,
  servers: [],
  runningServers: [],
  startServers: [],
  stoppedServers: [],
  runningServersHeader: ["Hostname", "Login", "Remote Directory", "Local Directory", "Status"],
  startServersHeader:   ["Hostname", "Login", "Remote Directory", "Local Directory", "Status"],
  stoppedServersHeader: ["Hostname", "Login", "Remote Directory", "Local Directory", "Status"],
  index: {"hostname":0, "login": 1, "remotedir": 2, "localdir": 3, "status":4},
  getLocalDirectory : function(file_entry_list) {
    var dir = process.env.AFNETWORK+"/"+file_entry_list[this.index["hostname"]]+"/"+file_entry_list[this.index["remotedir"]];
    dir = dir.replace(/:/g, "")
    return dir;
  },
  readServerFile : function(filename) {
    var file_contents = read.sync(filename, 'utf8');
    var server_list = []
    log.debug("Reading file " + filename);
    for(file_entry of file_contents.split("\n")) {
      file_entry = file_entry.trim();
      if(file_entry.length == 0) continue;
      var file_entry_list = file_entry.split(" ");
      var expected_length = Object.keys(this.index).length;
      if (file_entry_list[this.index["localdir"]] == undefined) {
        file_entry_list = file_entry_list.concat([this.getLocalDirectory(file_entry_list)]);
        log.debug("Adding column for localdir: "+ file_entry_list[this.index["localdir"]]);
      }
      if (file_entry_list[this.index["status"]] == undefined) {
        file_entry_list = file_entry_list.concat(["unknown"]);
        log.debug("Adding column for status: "+ file_entry_list[this.index["status"]]);
        //log.debug(file_entry_list);
      }
      if (file_entry_list.length == expected_length) {
        server_list.push(file_entry_list);
      } else {
        log.error("Entry in " + filename + " invalid ignoring: " + file_entry_list);
        log.error(file_entry_list.length);
        log.error(expected_length);
      }
    }
    return server_list;
  },
  writeServerFile : function(filename, contents) {
    var file_contents = ""
    for(var i=0; i < contents.length; i++) {
      file_contents += contents[i].join(' ')+'\n';
    }
    try {
      const data = fs.writeFileSync(filename, file_contents)
      log.debug("writeServerFile("+filename+") ok");
    } catch (err) {
      log.error(err)
    }
  },
  addServerEntryAsync : function(filename, value) {
    fs.appendFile(filename, value, (err) => {
      if (err) log.error(err);
      log.debug("Added=" + value + " to filename=" + filename);
    });
  },
  addServerEntry : function(filename, value) {
    try {
      fs.appendFileSync(filename, value);
      log.debug("Added=" + value + " to filename=" + filename);
    } catch(err) {
      log.error(err);
    }
  },
  getConfiguredServers : function() {
    runningfile = process.env.AFHOME + "/" + process.env.AF_TEXT_RUNNING,
    startfile = process.env.AFHOME + "/" + process.env.AF_TEXT_START,
    stoppedfile = process.env.AFHOME + "/" + process.env.AF_TEXT_STOP,
    this.runningServers = this.readServerFile(this.runningfile);
    this.startServers = this.readServerFile(this.startfile);
    this.stoppedServers = this.readServerFile(this.stoppedfile);
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
    var entry = afnet.stoppedServers[req.query.index].join(' ')+'\n';
    afnet.stoppedServers.splice(req.query.index, 1);
    afnet.writeServerFile(afnet.stoppedfile, afnet.stoppedServers);
    log.info("Removing an entry from the stop file: " + entry);
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

  if (req.query.indexname === "start") {
    afnet.startMessage = message;
    var entry = afnet.startServers[req.query.index].slice(0,3).join(' ')+'\n';
    afnet.addServerEntry(afnet.stoppedfile, entry);
    log.debug(message);
    log.info("Adding an entry to the stop file: " + entry);
  }

  res.render('run.ejs', {
    title: 'AFNET Home Stop',
    afnet: afnet,
  });
});

/* Main: express server ************************/

const server = app.listen(7000, () => {
  console.log(`Express running → PORT ${server.address().port}`);
  log.info(`Express running → PORT ${server.address().port}`);
});
