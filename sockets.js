var net = require('net');
var μs = require('microseconds');
var delay = 0;
var sendBack = false;
const NODE_ID = 2751889593;
var idList = []

function isExist(id) {
  for (var i = 0; i< myNodes.length; i++) {
      if (idList[i] == id) {
        return true
      }
  }
  return false
}

function subs(subset, id) {
  if(!isExist(id)) {
    idList.push(id);
  }
  if (subset.length == 0) {
    return;
  }
  for (var i = 0; i < subset.length; i++) {
    subs(subset[i]);
  }
  return;
}

function isJson(str) {
    try {
        JSON.parse(str);
    } catch (e) {
        return false;
    }
    return true;
}

function getType(str) {
  var obj;
  for (var i = 0; i < str.length; i++) {
    if (str.charAt(i) == '\0') {
        str = str.substring(0,i);
        var jobj = JSON.parse(str);
        console.log("this is the json");
        console.log(jobj);
        return jobj;
    }
  }
}
function timeSync(json) {
  var time = μs.now();
  if (json.type == 4) {
      if (json.msg.type == 1) {
        var t = json.msg.t0;
        var obj = {dest:json.from, from:NODE_ID, type:4, msg:
            { type: 2,
              t0: t,
              t1: time,
              t2: (time - μs.now()) + t
            }
          }
          sendBack = true;
          return obj;
      } else if (json.msg.type == 2) {
        delay = (time - json.msg.t0) - (json.msg.t2 - json.msg.t1)
        sendBack = false;
        return
      }
  }

  if (json.type == 6) {
    var obj = {dest:json.from, from:NODE_ID, type:4,
      msg: {
          type: 0,
          t0: time,
        }
      }
      sendBack = true;
      return obj;
  }
}

function typeHandler(json) {
  var msg;
  console.log(json.type)
  switch (json.type) {
    case 5:
      msg = {
              dest: json.from,
              from: NODE_ID,
              type: 6,
              subs: []
              }
      //var obj = JSON.stringify(msg);
      var toSend = JSON.stringify(msg);
      //console.log("We send: ", obj)
      sendBack = true;
      return toSend
      break;
      //end of case 5
      case 4 || 6:
        msg = timeSync(json)
        var toSend = JSON.stringify(msg);
        return toSend
        break;
      case 8 || 9:
        //msg = sendBoardCast("DAMN")
        sendBack = false
        console.log(json.msg);
        retreiveMessage(json.msg);
        break;
    default:
      sendBack = false
      return
  }
}

function retreiveMessage(obj) {
  //var obj = getType(msg)
  //console.log(obj)
  console.log(obj.topic)
  //console.log(obj.message.lat)
}

function sendBoardCast(dest, msg) {
  var obj = {
      dest: dest,
      from: NODE_ID,
      type: 8,
      msg: msg,
    }
    sendBack = true;
    var toSend = JSON.stringify(obj);
    return toSend;
}

function sleep(microseconds) {
  var start = μs.now();
  while (μs.now()-start < microseconds) {

  }
}

function connHandler(socket) {
    socket.on('data', function(data) {  // client writes message
      //console.log(JSON.parse(data))
      try {
        var myObj;
        str = data.toString()
        myObj = getType(str);
        //console.log("after type ", myObj)
        myObj = typeHandler(myObj);
        if (sendBack) {
          socket.write(myObj+'\0');
          sleep(delay+ 1000);
          myObj = sendBoardCast(0,"DAMN");
          socket.write(myObj+'\0');
          sleep(delay);
        }
      } catch (e) {
        console.log(e)
      }

    });

    socket.on('end', function() {
        console.log('Connection Closed !');
    });

    socket.on('error', function(error) {
        console.log('Connection Error: %s', error.message);
    });
}
try {
  net.createServer(connHandler).listen(5555);
  //net.createServer(connHandler).listen(5556);
  //net.createServer(connHandler).listen(5557);
} catch (e) {
  console.log(e)
}