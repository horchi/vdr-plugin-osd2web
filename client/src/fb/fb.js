/*
 *  fb.js
 *
 *  (c) 2023 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 */

var rowCount = 9;
var screenHeight = 0;

window.WebSocketClient = function(opt)
{
   if (!window.WebSocket)
      return false;

   var client = this;
   var queue = [];

   this.onclose = function () {
      console.log("websocket connection closed");
   }
   this.onerror = function (err) {
      console.log("websocket error: ", err);
   }
   for (var key in opt)
      this[key] = opt[key];
   this.reconnect = function (e) {
      if (client.autoReconnectInterval) {
         console.log(`WebSocketClient: retry in ${client.autoReconnectInterval}ms`, e);
         setTimeout(function () {
            console.log("WebSocketClient: reconnecting...");
            client.open();
         }, client.autoReconnectInterval);
      }
   }
   this.send = function (JSONobj) {
      if (queue)
         queue.push(JSONobj);
      else
         client.ws.send(JSON.stringify(JSONobj));
   }
   this.open = function () {
      client.ws = new WebSocket(client.url, client.protocol);
      client.ws.onopen = function(e){
         if (queue) {
            var JSONobj;
            while ((JSONobj=queue.shift()))
               client.ws.send(JSON.stringify(JSONobj));
            queue = null;
         }
         client.onopen && client.onopen(e);
      }
      client.ws.onmessage = client.onmessage;
      client.ws.onclose = function (e) {
         queue= [];
         switch (e) {
         case 1000:  // CLOSE_NORMAL
            console.log("WebSocket: closed");
            break;
         default:    // Abnormal closure
            client.reconnect(e);
            break;
         }
         client.onclose && client.onclose(e);
      };
      client.ws.onerror = function (e) {
         switch (e.code) {
         case 'ECONNREFUSED':
            client.reconnect(e);
            break;
         default:
            client.onerror || client.onerror(e);
            break;
         }
      };
   }
   this.close = function () {
      client.ws.close();
   }
   this.reopen = function () {
      client.ws.onerror = client.ws.onclose = client.ws.onmessage = null;
      client.ws.close();
      client.open();
   }
   this.open();
   return this;
}

var socket = null;
var isActive = null;

function onSocketConnect()
{
   function autoResizeDiv()
   {
      screenHeight = window.innerHeight -20;
      document.getElementById('content').style.height = screenHeight +'px';
      // console.log("set height to", screenHeight +'px');
      resizeButtons();
   }
   window.onresize = autoResizeDiv;
   autoResizeDiv();

   socket.send({ "event" : "login", "object" :
                 { "type" : "active",
                   "url" : window.location.href }
               });
}

function dispatchMessage(message)
{
   var jMessage = JSON.parse(message);
   var event = jMessage.event;

   // console.log("<- event (" + event + ")");

   if (event == 'commands')
   {
      commands = jMessage.object;
      console.log("commands", JSON.stringify(commands));
   }
}

function connectWebSocket()
{
   protocol = "osd2vdr";
   url = "ws://" + location.hostname + ":" + location.port;
   console.log("try to open socket", url, protocol);

   socket = new WebSocketClient({
      'url': url,
      'protocol': protocol,
      'autoReconnectInterval': 1000,
      onopen: function() {
         console.log("connected");
         if (isActive === null)     // wurde beim Schliessen auf null gesetzt
            onSocketConnect();
      },
      onclose: function () {
         isActive = null;           // auf null setzen, dass ein neues login aufgerufen wird
      },
      onmessage: function (msg) {
         dispatchMessage(msg.data)
      }.bind(this)
   });

   if (!socket)
      return !($el.innerHTML = "Your Browser will not support Websockets!");
}

function initVdr()
{
   connectWebSocket();
   initFb();
}

function initFb()
{
   document.getElementById("container").innerHTML =
      '<div class="vdrContent">' +
      '  <div id="vdrFbContainer" class="vdrFbContainer">' +
      '    <div class="vdrButtonDiv"><button class="vdrButton" type="button" onclick="vdrKeyPress(\'1\')">1</button></div>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton" type="button" onclick="vdrKeyPress(\'2\')">2</button></div>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton" type="button" onclick="vdrKeyPress(\'3\')">3</button></div>' +
      '    <br/>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton" type="button" onclick="vdrKeyPress(\'4\')">4</button></div>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton" type="button" onclick="vdrKeyPress(\'5\')">5</button></div>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton" type="button" onclick="vdrKeyPress(\'6\')">6</button></div>' +
      '    <br/>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton" type="button" onclick="vdrKeyPress(\'7\')">7</button></div>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton" type="button" onclick="vdrKeyPress(\'8\')">8</button></div>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton" type="button" onclick="vdrKeyPress(\'9\')">9</button></div>' +

      '    <br/>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton vdrButtonRound glyphicon glyphicon-volume-up" type="button" onclick="vdrKeyPress(\'volume-\')">🔉</button></div>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton" type="button" onclick="vdrKeyPress(\'\0\')">0</button></div>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton vdrButtonRound" type="button" onclick="vdrKeyPress(\'volume+\')">🔊</button></div>' +

      '    <br/>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton vdrButtonRound" type="button" onclick="vdrKeyPress(\'menu\')">&#9776;</button></div>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton vdrArrowButton vdrArrowButtonUp" type="button" onclick="vdrKeyPress(\'up\')">&uparrow;</button></div>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton vdrButtonRound" type="button" onclick="vdrKeyPress(\'back\')">↵</button></div>' +
      '    <br/>' +

      '    <div class="vdrButtonDiv"><button class="vdrButton vdrArrowButton vdrArrowButtonLeft" type="button" onclick="vdrKeyPress(\'left\')">&leftarrow;</button></div>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton vdrArrowButton vdrButtonRound" type="button" onclick="vdrKeyPress(\'ok\')">ok</button></div>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton vdrArrowButton vdrArrowButtonRight" type="button" onclick="vdrKeyPress(\'right\')">&rightarrow;</button></div>' +
      '    <br/>' +

      '    <div class="vdrButtonDiv"><button class="vdrButton vdrButtonRound" type="button" onclick="vdrKeyPress(\'mute\')">🔇</button></div>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton vdrArrowButton vdrArrowButtonDown" type="button" onclick="vdrKeyPress(\'down\')">&downarrow;</button></div>' +
      '    <div class="vdrButtonDiv"><button class="vdrButton vdrButtonRound" type="button" onclick="vdrKeyPress(\'info\')">ℹ</button></div>' +
      '    <br/>' +

      '    <div class="vdrButtonColorDiv"><button class="vdrButton vdrColorButtonRed" type="button" onclick="vdrKeyPress(\'red\')"></button></div>' +
      '    <div class="vdrButtonColorDiv"><button class="vdrButton vdrColorButtonGreen" type="button" onclick="vdrKeyPress(\'green\')"></button></div>' +
      '    <div class="vdrButtonColorDiv"><button class="vdrButton vdrColorButtonYellow" type="button" onclick="vdrKeyPress(\'yellow\')"></button></div>' +
      '    <div class="vdrButtonColorDiv"><button class="vdrButton vdrColorButtonBlue" type="button" onclick="vdrKeyPress(\'blue\')"></button></div>' +
      '    <br/>' +
      '    <div class="vdrCommandButtonDiv"><button class="vdrButton vdrCommandButton" type="button" onclick="vdrCommands()">Commands</button></div>' +
      '  </div>' +
      '</div>';

   resizeButtons();
}

function resizeButtons()
{
   var root = document.getElementById("vdrFbContainer");
   var elements = root.getElementsByClassName("vdrButtonDiv");
   let buttonHeight = screenHeight / rowCount;

   console.log("set button height to", buttonHeight);

   for (var i = 0; i < elements.length; i++) {
      elements[i].style.height = buttonHeight + 'px'; // getComputedStyle(elements[i]).width;
      if (elements[i].children[0].innerHTML == '')
         elements[i].style.visibility = 'hidden';
   }
}

function vdrKeyPress(key)
{
   if (key == undefined || key == "")
      return;

   console.log("sending key", key);

   socket.send({ 'event' : 'keypress',
                 'object' : {
                    'key' : key,
                    'repeat' : 1
                 }
               });
}

function vdrCommandSelect(command)
{
   socket.send({ 'event' : 'command',
                 'object' : {
                    'command' : command
                 }
               });
}

function vdrCommands()
{
   let html = '<div class="vdrContent">' +
       '  <div id="vdrFbContainer" class="vdrFbContainer">' +
       '    <div class="vdrCommandButtonDiv"><button class="vdrButton vdrCommandButton" type="button" onclick="initFb()">' + '<<' + '</button></div>';

   for (var key in commands) {
      html += '    <div class="vdrCommandButtonDiv"><button class="vdrButton vdrCommandButton" type="button" onclick="vdrCommandSelect(\'' + commands[key] + '\')">' + key + '</button></div>';
   }

   html += '  </div>' +
      '</div>';

   document.getElementById("container").innerHTML = html;
}
