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

log.head = function(value) {
  log.info("");
  log.info("----------[   " + value + "   ]----------");
};

/* Objects ************************/
var current_date = new Date();
var afnet = {
  //ui_message : current_date.getTime(),
  ui_message : "It is "+current_date.toUTCString(),
  ui_server_errors: "",
  ui_server_info_list: [],
  // keep ui_index and the these arrays in sync, it is not too much to ask, do not show this to your peers
  ui_server_info_header: ["Index", "Status", "Hostname", "Login", "Remote Directory", "Local Directory", "si_line"],
  ui_server_info_header_required: ["(Index)", "(Status)", "Hostname", "Login", "Remote Directory", "(Local Directory)", "(si_line)"],
  ui_server_info_header_varname: ["index", "status", "hostname", "login", "remotedir", "localdir", "si_line"],
  ui_index: {"index": 0, "hostname":2, "login": 3, "remotedir": 4, "localdir": 5, "status":1, "si_line": 6},
  // keep this is sync with the file contents
  si_file_header: ["hostname", "login", "remotedir", "localdir"],
  si: {},
  errorsfile : process.env.AFHOME + "/" + process.env.AF_TEXT_ERRORS,
  runningfile : process.env.AFHOME + "/" + process.env.AF_TEXT_RUNNING,
  startfile : process.env.AFHOME + "/" + process.env.AF_TEXT_START,
  stoppedfile : process.env.AFHOME + "/" + process.env.AF_TEXT_STOP,
  deletefile : process.env.AFHOME + "/" + process.env.AF_TEXT_DELETE,

  getLocalDirectory : function(si_line) {
    var dir = process.env.AFNETWORK+"/"+si_line[this.ui_index["hostname"]]+"/"+si_line[this.ui_index["remotedir"]];
    dir = dir.replace(/:/g, "")
    return dir;
  },
  readServers : function() {
    log.head("readServers");
    log.info("Reading server information readServers()");
    this.si = {}
    this.ui_server_info_list=[];
    var server_list = []
    var expected_length = Object.keys(this.ui_index).length;
    // Read the files
    log.info("Reading start configuration file " + this.startfile);
    try {
      var file_contents_start = read.sync(this.startfile, 'utf8');
      log.info("Reading running configuration file " + this.runningfile);
      var file_contents_running = read.sync(this.runningfile, 'utf8');
      log.info("Reading stopped configuration file " + this.stoppedfile);
      var file_contents_stopped = read.sync(this.stoppedfile, 'utf8');
      log.info("Reading errors file " + this.errorsfile);
      this.ui_server_errors = "";
      var t  = read.sync(this.errorsfile, 'utf8');
      if (t.length > 0) this.ui_server_errors = "The following errors were recorded by the afnet-service: " + t;
      var index = -1;
    } catch (err) {
      log.error(err)
      return;
    }

    for(si_line of file_contents_start.split("\n")) {
      index += 1;
      si_line = si_line.trim();
      if(si_line.length == 0) continue;

      // split line create entry
      var si_line_split = si_line.split(" ");
      var si_values = []

      // add the values 
      si_values[ this.ui_index["hostname"] ] = si_line_split[0];
      si_values[ this.ui_index["login"] ] = si_line_split[1];
      si_values[ this.ui_index["remotedir"] ] = si_line_split[2];
      //
      si_values[ this.ui_index["localdir"] ] = this.getLocalDirectory(si_values);
      si_values[ this.ui_index["index"] ] = index;
      si_values[ this.ui_index["status"] ] = "Configured";
      si_values[ this.ui_index["si_line"] ] = si_line;

      // set status
      // TODO check if bash server is running if so, say "Starting"
      for(tline of file_contents_running.split("\n")) {
        tline = tline.trim();
        if(tline.length == 0) continue;
        log.debug(tline);
        log.debug(si_line);
        if (tline == si_line) {
          log.debug("SERVER: running");
          si_values[ this.ui_index["status"] ] = "Running";
        }
      }
      for(tline of file_contents_stopped.split("\n")) {
        tline = tline.trim();
        if(tline.length == 0) continue;
        if (tline == si_line) {
          log.debug("SERVER: stopped");
          si_values[ this.ui_index["status"] ] = "Stopped";
        }
      }

      // add the server
      //log.info(si_values);
      if (si_values.length == expected_length) {
        if (this.si[si_line] == undefined) {
          this.ui_server_info_list.push(si_values);
          this.si[si_line] = si_values;
          log.info("SERVER: Entry added: " + si_line);
        } else {
          log.error("SERVER: Entry ignored (duplicate): " + si_line);
        }
      } else {
        log.error("SERVER: Entry ignored (invalid): " + si_line);
        log.error(si_values.length);
        log.error(expected_length);
      }
    }
  },
  writeServerFile : function(filename, contents) {
    log.head("writeServerFile");
    try {
      const data = fs.writeFileSync(filename, contents);
      log.debug("writeServerFile("+filename+") ok");
    } catch (err) {
      log.error(err)
    }
  },
  addServerEntryAsync : function(filename, value) {
    log.head("addServerEntryAsync");
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
    log.head("addServerEntry");
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
    log.head("rmServerEntry");
    log.info("looking for '" + value + "'");
    try {
      var file_contents = read.sync(filename, 'utf8');
      var new_contents = ""
      for(si_line of file_contents.split("\n")) {
        si_line = si_line.trim();
        if(si_line.length == 0) continue;
        log.info("found '" + si_line + "'");
        if (si_line == value) {
          log.info("rmServerEntry removed entry: " + si_line);
          continue;
        }
        new_contents += si_line + "\n";
      }
      this.writeServerFile(filename, new_contents);
    } catch(err) {
      log.error("rmServerEntry failed for: " + filename);
      log.error("rmServerEntry failed for: " + value);
      log.error(err);
    }
  },
  getConfiguredServers : function() {
    log.head("getConfiguredServers");
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
  afnet.ui_message = "It is "+current_date.toUTCString();
});

app.get('/home', (req, res) => {
  afnet.getConfiguredServers();
  res.render('home.ejs', {
    title: 'AFNET Home',
    afnet: afnet,
  });
  afnet.ui_message = "It is "+current_date.toUTCString();
});

app.get('/start', (req, res) => {
  log.head("/start");
  afnet.ui_message = "Starting entry that was stopped " + req.query.index + " '"+ req.query.si_line + "'.";
  afnet.rmServerEntry(afnet.stoppedfile, req.query.si_line);
  res.render('run.ejs', {
    title: 'AFNET Home Start',
    afnet: afnet,
  });
});

app.get('/stop', (req, res) => {
  log.head("/stop");
  afnet.ui_message = "Stopping entry that was configured " + req.query.index + " '"+ req.query.si_line + "'.";
  afnet.addServerEntry(afnet.stoppedfile, req.query.si_line+"\n");
  res.render('run.ejs', {
    title: 'AFNET Home Stop',
    afnet: afnet,
  });
});

app.get('/add', (req, res) => {
  log.head("/add");
  //log.info(req);
  afnet.ui_message = "Adding new entry remote dir '" + req.query.remotedir + "'.";
  si_line = ""
  si_line += req.query.hostname;
  si_line += " ";
  si_line += req.query.login;
  si_line += " ";
  si_line += req.query.remotedir;
  log.info(si_line);
  afnet.addServerEntry(afnet.startfile, si_line+"\n");
  res.render('run.ejs', {
    title: 'AFNET Home Stop',
    afnet: afnet,
  });
});

app.get('/del', (req, res) => {
  log.head("/del");
  //log.info(req);
  afnet.ui_message = "Deleting entry that was stopped " + req.query.index + " '"+ req.query.si_line + "'.";
  afnet.rmServerEntry(afnet.startfile, req.query.si_line);
  afnet.addServerEntry(afnet.deletefile, req.query.si_line);
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
