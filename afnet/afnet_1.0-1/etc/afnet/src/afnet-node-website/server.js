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
var current_date = new Date();
var afnet = {
  //ui_message : current_date.getTime(),
  ui_message : current_date.toUTCString(),
  ui_server_info_list: [],
  ui_server_info_header: ["id", "Status", "Hostname", "Login", "Remote Directory", "Local Directory"],
  runningfile : process.env.AFHOME + "/" + process.env.AF_TEXT_RUNNING,
  startfile : process.env.AFHOME + "/" + process.env.AF_TEXT_START,
  stoppedfile : process.env.AFHOME + "/" + process.env.AF_TEXT_STOP,
  server_info: {},
  index: {"id": 0, "hostname":2, "login": 3, "remotedir": 4, "localdir": 5, "status":1},

  getLocalDirectory : function(file_entry_list) {
    var dir = process.env.AFNETWORK+"/"+file_entry_list[this.index["hostname"]]+"/"+file_entry_list[this.index["remotedir"]];
    dir = dir.replace(/:/g, "")
    return dir;
  },
  readServers : function() {
    log.info("Reading server information readServers()");
    this.server_info = {}
    this.ui_server_info_list=[];
    var server_list = []
    var expected_length = Object.keys(this.index).length;
    // Read the files
    var file_contents_start = read.sync(this.startfile, 'utf8');
    var file_contents_running = read.sync(this.runningfile, 'utf8');
    var file_contents_stopped = read.sync(this.stoppedfile, 'utf8');

    for(server_line of file_contents_start.split("\n")) {
      server_line = server_line.trim();
      if(server_line.length == 0) continue;

      // split line
      var server_values = [server_line, "Starting"]
      server_values = server_values.concat(server_line.split(" "));
      server_values = server_values.concat([this.getLocalDirectory(server_values)]);

      // check status:
      for(tline of file_contents_running.split("\n")) {
        tline = tline.trim();
        if(tline.length == 0) continue;
        log.debug(tline);
        log.debug(server_line);
        if (tline == server_line) {
          log.debug("SERVER: running");
          server_values[ this.index["status"] ] = "Running";
        }
      }
      for(tline of file_contents_stopped.split("\n")) {
        tline = tline.trim();
        if(tline.length == 0) continue;
        if (tline == server_line) {
          log.debug("SERVER: stopped");
          server_values[ this.index["status"] ] = "Stopped";
        }
      }

      // add the server
      //log.info(server_values);
      if (server_values.length == expected_length) {
        if (this.server_info[server_line] == undefined) {
          this.ui_server_info_list.push(server_values);
          this.server_info[server_line] = server_values;
          log.info("SERVER: Entry added: " + server_line);
        } else {
          log.error("SERVER: Entry ignored (duplicate): " + server_line);
        }
      } else {
        log.error("SERVER: Entry ignored (invalid): " + server_line);
      }
    }
  },
  writeServerFile : function(filename, contents) {
    try {
      const data = fs.writeFileSync(filename, contents);
      log.debug("writeServerFile("+filename+") ok");
    } catch (err) {
      log.error(err)
    }
  },
  addServerEntryAsync : function(filename, value) {
    fs.appendFile(filename, value, (err) => {
      if (err) {
        log.error("addServerEntryAsync failed for: " + filename);
        log.error("addServerEntryAsync failed for: " + value);
        log.error(err);
      } else {
        log.debug("Added='" + value + "' to filename=" + filename);
      }
    });
  },
  addServerEntry : function(filename, value) {
    try {
      fs.appendFileSync(filename, value);
      log.debug("Added= '" + value + "' to filename=" + filename);
    } catch(err) {
      log.error("addServerEntry failed for: " + filename);
      log.error("addServerEntry failed for: " + value);
      log.error(err);
    }
  },
  rmServerEntry : function(filename, value) {
    try {
      var file_contents = read.sync(filename, 'utf8');
      var new_contents = ""
      for(server_line of file_contents.split("\n")) {
        if (server_line == value) {
          log.info("rmServerEntry removed entry: " + server_line);
          continue;
        }
        new_contents += server_line + "\n";
      }
      this.writeServerFile(filename, new_contents);
    } catch(err) {
      log.error("rmServerEntry failed for: " + filename);
      log.error("rmServerEntry failed for: " + value);
      log.error(err);
    }
  },
  getConfiguredServers : function() {
    log.info("Reading the servers files");
    runningfile = process.env.AFHOME + "/" + process.env.AF_TEXT_RUNNING,
    startfile = process.env.AFHOME + "/" + process.env.AF_TEXT_START,
    stoppedfile = process.env.AFHOME + "/" + process.env.AF_TEXT_STOP,
    this.readServers();
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
  afnet.ui_message = "Starting entry that was stopped " + req.query.index + " '"+ req.query.server_line + "'.";
  afnet.rmServerEntry(afnet.stoppedfile, req.query.server_line);
  res.render('run.ejs', {
    title: 'AFNET Home Start',
    afnet: afnet,
  });
});

app.get('/stop', (req, res) => {
  afnet.ui_message = "Stopping entry that was configured " + req.query.index + " '"+ req.query.server_line + "'.";
  afnet.addServerEntry(afnet.stoppedfile, req.query.server_line+"\n");
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
