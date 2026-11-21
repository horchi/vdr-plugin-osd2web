/*
 *  fb.js
 *
 *  (c) 2023-2026 Jörg Wendel
 *
 * This code is distributed under the terms and conditions of the
 * GNU GENERAL PUBLIC LICENSE. See the file COPYING for details.
 *
 */

// SVG icons (stroke based, follow currentColor)

const vdrIcons = {
   menu:  '<path d="M4 7h16M4 12h16M4 17h16"/>',
   info:  '<circle cx="12" cy="12" r="9"/><path d="M12 11v6M12 7.5v.01"/>',
   back:  '<path d="M9 14l-5-5 5-5"/><path d="M4 9h10a6 6 0 0 1 0 12h-3"/>',
   up:    '<path d="M6 15l6-6 6 6"/>',
   down:  '<path d="M6 9l6 6 6-6"/>',
   left:  '<path d="M15 6l-6 6 6 6"/>',
   right: '<path d="M9 6l6 6-6 6"/>',
   plus:  '<path d="M12 5v14M5 12h14"/>',
   minus: '<path d="M5 12h14"/>',
   mute:  '<path d="M4 9v6h4l5 4V5L8 9z"/><path d="M16 9l5 6M21 9l-5 6"/>',
   list:  '<path d="M9 6h11M9 12h11M9 18h11"/><path d="M4.5 6h.01M4.5 12h.01M4.5 18h.01"/>',
   close: '<path d="M6 6l12 12M18 6L6 18"/>',
   timer: '<circle cx="12" cy="13" r="8"/><path d="M12 9v4l2.5 2.5M9 2h6"/>',
   cmd:   '<path d="M5 7l5 5-5 5"/><path d="M12 17h7"/>',
   num:   '<circle cx="6" cy="6" r="1.3"/><circle cx="12" cy="6" r="1.3"/><circle cx="18" cy="6" r="1.3"/>' +
          '<circle cx="6" cy="12" r="1.3"/><circle cx="12" cy="12" r="1.3"/><circle cx="18" cy="12" r="1.3"/>' +
          '<circle cx="6" cy="18" r="1.3"/><circle cx="12" cy="18" r="1.3"/><circle cx="18" cy="18" r="1.3"/>'
};

var commands = {};
var vdrActual = null;
var vdrSocket = null;
var vdrOnline = false;
var vdrProgressTimer = null;
var vdrEpgView = null;         // open overlay { mode: 'now'|'schedule'|'commands', channelid }

// embedded in the osd2web web frontend (fb.html?embedded) -> the menu key brings the OSD into
//   the browser, standalone it's a remote control for the TV and must not attach the osd2web skin

var vdrEmbedded = /[?&]embedded/.test(location.search);

//***************************************************************************
// Web Socket Client
//***************************************************************************

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
      client.ws.onopen = function(e) {
         if (queue) {
            var JSONobj;
            while ((JSONobj = queue.shift()))
               client.ws.send(JSON.stringify(JSONobj));
            queue = null;
         }
         client.onopen && client.onopen(e);
      }
      client.ws.onmessage = client.onmessage;
      client.ws.onclose = function (e) {
         queue = [];
         if (e.code != 1000)    // not CLOSE_NORMAL
            client.reconnect(e);
         client.onclose && client.onclose(e);
      };
      client.ws.onerror = function (e) {
         client.onerror && client.onerror(e);
      };
   }
   this.close = function () {
      client.ws.close();
   }
   this.open();
   return this;
}

function connectWebSocket()
{
   const url = "ws://" + location.hostname + ":" + location.port;
   console.log("try to open socket", url);

   vdrSocket = new WebSocketClient({
      url: url,
      protocol: 'osd2vdr',
      autoReconnectInterval: 2000,
      onopen: function() {
         vdrOnline = true;
         vdrUpdateState();
         vdrSocket.send({ "event" : "login", "object" :
                          { "type" : 2,       // ctFB -> pure remote control client
                            "url" : window.location.href }
                        });
      },
      onclose: function() {
         vdrOnline = false;
         vdrUpdateState();
      },
      onmessage: function(msg) {
         dispatchMessage(JSON.parse(msg.data));
      }
   });

   if (!vdrSocket)
      $el('container').innerHTML = "Your Browser will not support Websockets!";
}

function dispatchMessage(jMessage)
{
   const event = jMessage.event;

   if (event == 'commands') {
      commands = jMessage.object || {};
      $show('vdrCmdKey', Object.keys(commands).length > 0);
   }
   else if (event == 'actual') {
      vdrActual = jMessage.object;
      showActual(vdrActual);
   }
   else if (event == 'epg') {
      vdrEpgShow(jMessage.object);
   }
}

//***************************************************************************
// Helper
//***************************************************************************

function $el(id)
{
   return document.getElementById(id);
}

function $show(id, show)
{
   const elem = $el(id);

   if (elem)
      elem.style.display = show ? '' : 'none';
}

function setHtml(id, html)
{
   const elem = $el(id);

   if (elem)
      elem.innerHTML = html || '';
}

function vdrEscape(s)
{
   return String(s == undefined ? '' : s).replace(/[&<>"']/g, c =>
      ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' })[c]);
}

function timeOf(starttime)
{
   if (!starttime)
      return '';

   const date = new Date(starttime * 1000);
   const ten = function(i) { return (i < 10 ? '0' : '') + i; };

   return ten(date.getHours()) + ':' + ten(date.getMinutes());
}

function vdrIcon(name)
{
   return '<svg class="vdrIcon" viewBox="0 0 24 24" aria-hidden="true">' + vdrIcons[name] + '</svg>';
}

function vdrHaptic()
{
   if (navigator.vibrate)
      navigator.vibrate(8);
}

// osd2web serves the logos by HTTP on the port of its websocket (same origin as this page)

function vdrLogoUrl(channel)
{
   return '/data/channellogo?name=' + encodeURIComponent(channel.channelname) +
      '&id=' + encodeURIComponent(channel.channelid || '');
}

// key button, 'repeat' keys repeat while pressed

function vdrKey(key, content, cls, repeat, title)
{
   return '<button type="button" class="vdrKey ' + (cls || '') + '" data-key="' + key + '"' +
      (repeat ? ' data-repeat="1"' : '') + ' title="' + (title || key) + '">' + content + '</button>';
}

//***************************************************************************
// Init
//***************************************************************************

function initVdr()
{
   let num = '';

   if (vdrEmbedded)
      document.documentElement.classList.add('vdrEmbedded');

   for (const k of ['1','2','3','4','5','6','7','8','9'])
      num += vdrKey(k, k, 'vdrNumKey');

   num += '<span></span>' + vdrKey('0', '0', 'vdrNumKey') + '<span></span>';

   $el("container").innerHTML =
      '<div class="vdrContent">' +
      '  <div class="vdrPresent" onclick="vdrEpgOpen(\'schedule\')" title="Programm des Kanals">' +
      '    <div class="vdrChannel">' +
      '      <img id="vdrChannelLogo" class="vdrChannelLogo" style="display:none;" onerror="this.style.display=\'none\'" onload="this.style.display=\'\'"/>' +
      '      <div id="vdrChannel" class="vdrChannelName"></div>' +
      '    </div>' +
      '    <div class="vdrEpg">' +
      '      <div class="vdrEpgLine"><span id="vdrStartTime" class="vdrTime"></span><span id="vdrTitle" class="vdrTitle"></span></div>' +
      '      <div id="vdrShorttext" class="vdrShorttext"></div>' +
      '      <div class="vdrProgress"><div id="vdrProgressBar"></div></div>' +
      '      <div class="vdrEpgLine vdrEpgNext"><span id="vdrStartTimeNext" class="vdrTime"></span><span id="vdrTitleNext" class="vdrTitle"></span></div>' +
      '    </div>' +
      '  </div>' +
      '  <div id="vdrRemote" class="vdrRemote">' +
      '    <div class="vdrHeadRow">' +
      '      <span class="vdrSelectWrap vdrSingle">' +
      '        <span id="vdrState" class="vdrState"></span>' +
      '        <span class="vdrName">' + vdrEscape(location.hostname) + '</span>' +
      '      </span>' +
      '      <button type="button" id="vdrCmdKey" class="vdrKey vdrRound vdrCmdKey" title="Befehle" style="display:none;" onclick="vdrCommandsOpen()">' + vdrIcon('cmd') + '</button>' +
      '    </div>' +
      '    <div class="vdrFnRow">' +
             vdrKey('Menu', vdrIcon('menu'), 'vdrRound', false, 'Menü') +
             vdrKey('Info', vdrIcon('info'), 'vdrRound', false, 'Info') +
             vdrKey('Back', vdrIcon('back'), 'vdrRound', false, 'Zurück') +
      '      <button type="button" class="vdrKey vdrRound vdrEpgKey" title="Was läuft jetzt" onclick="vdrEpgOpen(\'now\')">' + vdrIcon('list') + '</button>' +
      '      <button type="button" class="vdrKey vdrRound vdrNumToggle" title="Ziffern" onclick="vdrToggleNum()">' + vdrIcon('num') + '</button>' +
      '    </div>' +
      '    <div class="vdrPadArea">' +
      '      <div class="vdrDpad">' +
               vdrKey('Up', vdrIcon('up'), 'vdrWedge vdrWedgeUp', true) +
               vdrKey('Down', vdrIcon('down'), 'vdrWedge vdrWedgeDown', true) +
               vdrKey('Left', vdrIcon('left'), 'vdrWedge vdrWedgeLeft', true) +
               vdrKey('Right', vdrIcon('right'), 'vdrWedge vdrWedgeRight', true) +
               vdrKey('Ok', 'OK', 'vdrOk') +
      '      </div>' +
      '    </div>' +
      '    <div class="vdrRockRow">' +
      '      <div class="vdrRocker">' +
               vdrKey('Volume+', vdrIcon('plus'), '', true, 'Lauter') +
      '        <span class="vdrRockerLabel">VOL</span>' +
               vdrKey('Volume-', vdrIcon('minus'), '', true, 'Leiser') +
      '      </div>' +
      '      <div class="vdrRockMid">' +
               vdrKey('Mute', vdrIcon('mute'), 'vdrRound', false, 'Stumm') +
      '      </div>' +
      '      <div class="vdrRocker">' +
               vdrKey('Channel+', vdrIcon('up'), '', true, 'Kanal +') +
      '        <span class="vdrRockerLabel">CH</span>' +
               vdrKey('Channel-', vdrIcon('down'), '', true, 'Kanal -') +
      '      </div>' +
      '    </div>' +
      '    <div class="vdrNumPad">' + num + '</div>' +
      '    <div class="vdrColorRow">' +
             vdrKey('Red', '', 'vdrColor vdrRed', false, 'Rot') +
             vdrKey('Green', '', 'vdrColor vdrGreen', false, 'Grün') +
             vdrKey('Yellow', '', 'vdrColor vdrYellow', false, 'Gelb') +
             vdrKey('Blue', '', 'vdrColor vdrBlue', false, 'Blau') +
      '    </div>' +
      '  </div>' +  // vdrRemote
      '  <div id="vdrEpgList" class="vdrEpgList" style="display:none;">' +
      '    <div class="vdrEpgHead">' +
      '      <button type="button" id="vdrEpgBack" class="vdrKey vdrRound" title="Zurück" onclick="vdrEpgBack()">' + vdrIcon('back') + '</button>' +
      '      <div id="vdrEpgTitle" class="vdrEpgTitle"></div>' +
      '      <button type="button" class="vdrKey vdrRound" title="Schließen" onclick="vdrEpgClose()">' + vdrIcon('close') + '</button>' +
      '    </div>' +
      '    <div id="vdrEpgRows" class="vdrEpgRows"></div>' +
      '  </div>' +
      '</div>';

   $el('vdrEpgRows').addEventListener('click', vdrEpgClick);
   vdrInitKeys($el('vdrRemote'));
   vdrUpdateState();

   vdrProgressTimer = setInterval(vdrUpdateProgress, 30000);

   connectWebSocket();
}

function vdrToggleNum()
{
   $el('vdrRemote').classList.toggle('vdrShowNum');
   vdrHaptic();
}

function vdrUpdateState()
{
   const remote = $el('vdrRemote');

   if (!remote)
      return;

   remote.dataset.state = vdrOnline ? 'online' : 'offline';
   $el('vdrState').title = vdrOnline ? 'verbunden' : 'nicht verbunden';

   if (!vdrOnline)
      vdrEpgClose();
}

// pointer handling: send on press, keys with data-repeat repeat while hold

function vdrInitKeys(remote)
{
   let timer = null;

   const stop = function() {
      if (timer) {
         clearTimeout(timer);
         timer = null;
      }
   };

   remote.addEventListener('pointerdown', function(ev) {
      const btn = ev.target.closest('.vdrKey[data-key]');

      if (!btn || ev.button > 0)
         return;

      ev.preventDefault();
      stop();
      vdrHaptic();
      vdrKeyPress(btn.dataset.key);

      if (btn.dataset.repeat) {
         const repeat = function() {
            vdrKeyPress(btn.dataset.key);
            timer = setTimeout(repeat, 150);
         };
         timer = setTimeout(repeat, 450);
      }
   });

   for (const evName of ['pointerup', 'pointercancel', 'pointerleave'])
      remote.addEventListener(evName, stop);

   remote.addEventListener('pointerout', function(ev) {
      if (ev.target.closest('.vdrKey[data-repeat]'))
         stop();
   });

   remote.addEventListener('contextmenu', function(ev) { ev.preventDefault(); });
}

function vdrKeyPress(key)
{
   if (key == undefined || key == "" || !vdrSocket || !vdrOnline)
      return;

   vdrSocket.send({ "event": "keypress", "object": { "key": key, "repeat": 1, "focus": vdrEmbedded ? 1 : 0 } });
}

//***************************************************************************
// Present / Following
//***************************************************************************

function showActual(actual)
{
   const channel = actual.channel || {};
   const present = actual.present || {};
   const following = actual.following || {};

   setHtml('vdrChannel', vdrEscape(channel.channelname));
   vdrSetChannelLogo(channel);

   setHtml('vdrStartTime', timeOf(present.starttime));
   setHtml('vdrTitle', vdrEscape(present.title));
   setHtml('vdrShorttext', present.shorttext != present.title ? vdrEscape(present.shorttext) : '');

   setHtml('vdrStartTimeNext', timeOf(following.starttime));
   setHtml('vdrTitleNext', vdrEscape(following.title));

   vdrUpdateProgress();
   vdrEpgMarkCurrent();
}

function vdrSetChannelLogo(channel)
{
   const logo = $el('vdrChannelLogo');

   if (!logo)
      return;

   if (!channel || !channel.channelname) {
      logo.removeAttribute('src');
      logo.style.display = 'none';
      return;
   }

   const src = vdrLogoUrl(channel);

   if (logo.getAttribute('src') != src)
      logo.setAttribute('src', src);
}

function vdrUpdateProgress()
{
   const bar = $el('vdrProgressBar');

   if (!bar)
      return;

   const present = vdrActual ? vdrActual.present : null;

   if (!present || !present.starttime || !present.duration) {
      bar.parentNode.style.visibility = 'hidden';
      return;
   }

   let pct = (Date.now() / 1000 - present.starttime) * 100 / present.duration;
   pct = Math.max(0, Math.min(100, pct));
   bar.parentNode.style.visibility = '';
   bar.style.width = pct + '%';
}

//***************************************************************************
// EPG list (osd2web 'epg' request)
//   'now'      - what's on now, all channels; click on the logo -> schedule of the channel
//   'schedule' - events of one channel
//   click on an event switches to its channel
//***************************************************************************

function vdrEpgOpen(mode, channelid)
{
   if (!vdrSocket || !vdrOnline)
      return;

   vdrHaptic();

   if (mode == 'schedule' && !channelid)
      channelid = vdrActual && vdrActual.channel ? vdrActual.channel.channelid : '';

   vdrEpgView = { mode: mode, channelid: channelid || '' };

   setHtml('vdrEpgTitle', mode == 'now' ? 'Was läuft jetzt' : '');
   $el('vdrEpgBack').style.visibility = mode == 'schedule' ? 'visible' : 'hidden';
   setHtml('vdrEpgRows', '<div class="vdrEpgInfo">lade ...</div>');
   $show('vdrEpgList', true);

   let request = { "mode": mode };

   if (mode == 'schedule' && channelid)
      request.channelid = channelid;

   vdrSocket.send({ "event": "epg", "object": request });
}

function vdrEpgClose()
{
   vdrEpgView = null;
   $show('vdrEpgList', false);
   setHtml('vdrEpgRows', '');
}

function vdrEpgBack()
{
   if (vdrEpgView && vdrEpgView.mode == 'schedule')
      vdrEpgOpen('now');
   else
      vdrEpgClose();
}

function vdrEpgShow(epg)
{
   if (!vdrEpgView || !epg || epg.mode != vdrEpgView.mode)
      return;

   const now = Date.now() / 1000;
   const currentId = vdrActual && vdrActual.channel ? vdrActual.channel.channelid : '';
   const days = ['So', 'Mo', 'Di', 'Mi', 'Do', 'Fr', 'Sa'];
   let html = '';
   let lastDay = '';

   if (epg.mode == 'schedule') {
      if (!epg.channel || (vdrEpgView.channelid && epg.channel.channelid != vdrEpgView.channelid))
         return;

      vdrEpgView.channelid = epg.channel.channelid;
      setHtml('vdrEpgTitle', '<img class="vdrEpgTitleLogo" src="' + vdrLogoUrl(epg.channel) + '" onerror="this.remove()"/>' +
              '<span>' + vdrEscape(epg.channel.channelname) + '</span>');
   }

   for (const ev of epg.events || []) {
      const channel = epg.mode == 'now' ? ev.channel : epg.channel;

      if (!channel)
         continue;

      const start = new Date(ev.starttime * 1000);
      const running = ev.starttime <= now && ev.endtime > now;
      const timer = ev.timermatch == 'full' || ev.timermatch == 'partial' || ev.hastimer;
      const pct = running && ev.duration ? Math.max(0, Math.min(100, (now - ev.starttime) * 100 / ev.duration)) : 0;

      if (epg.mode == 'schedule') {
         const day = days[start.getDay()] + ' ' + start.getDate() + '.' + (start.getMonth() + 1) + '.';

         if (day != lastDay) {
            html += '<div class="vdrEpgDay">' + day + '</div>';
            lastDay = day;
         }
      }

      html += '<div class="vdrEpgRow' + (epg.mode == 'now' && channel.channelid == currentId ? ' vdrEpgCurrent' : '') +
         (running ? ' vdrEpgRunning' : '') + '" data-channelid="' + vdrEscape(channel.channelid) + '">';

      if (epg.mode == 'now')
         html += '<div class="vdrEpgLogo" data-schedule="' + vdrEscape(channel.channelid) + '" title="Programm von ' + vdrEscape(channel.channelname) + '">' +
            '<img loading="lazy" src="' + vdrLogoUrl(channel) + '" onerror="this.remove()"/>' +
            '<span>' + vdrEscape(channel.channelname) + '</span></div>';

      html += '<div class="vdrEpgEvent">' +
         '<div class="vdrEpgLine"><span class="vdrTime">' + timeOf(ev.starttime) +
         (epg.mode == 'now' ? ' - ' + timeOf(ev.endtime) : '') + '</span>' +
         '<span class="vdrTitle">' + vdrEscape(ev.title) + '</span>' +
         (timer ? '<span class="vdrEpgTimer' + (ev.timermatch == 'partial' ? ' vdrEpgPartial' : '') + '" title="Timer">' + vdrIcon('timer') + '</span>' : '') +
         '</div>' +
         (ev.shorttext && ev.shorttext != ev.title ? '<div class="vdrShorttext">' + vdrEscape(ev.shorttext) + '</div>' : '') +
         (running ? '<div class="vdrProgress"><div style="width:' + pct + '%"></div></div>' : '') +
         '</div></div>';
   }

   if (html == '')
      html = '<div class="vdrEpgInfo">keine EPG Daten</div>';

   const rows = $el('vdrEpgRows');
   rows.innerHTML = html;
   rows.scrollTop = 0;

   const current = rows.querySelector('.vdrEpgCurrent');

   if (current)
      current.scrollIntoView({ block: 'center' });
}

// mark the current channel in the 'now' list (after switching)

function vdrEpgMarkCurrent()
{
   if (!vdrEpgView || vdrEpgView.mode != 'now' || !vdrActual || !vdrActual.channel)
      return;

   for (const row of document.querySelectorAll('#vdrEpgRows .vdrEpgRow'))
      row.classList.toggle('vdrEpgCurrent', row.dataset.channelid == vdrActual.channel.channelid);
}

function vdrEpgClick(ev)
{
   const cmd = ev.target.closest('[data-command]');

   if (cmd) {
      vdrCommandExecute(cmd.dataset.command);
      return;
   }

   const logo = ev.target.closest('[data-schedule]');

   if (logo) {
      vdrEpgOpen('schedule', logo.dataset.schedule);
      return;
   }

   const row = ev.target.closest('.vdrEpgRow');

   if (!row || !row.dataset.channelid || !vdrSocket || !vdrOnline)
      return;

   vdrHaptic();
   vdrSocket.send({ "event": "switch", "object": { "channelid": row.dataset.channelid } });
}

//***************************************************************************
// Commands (shown in the overlay of the EPG list)
//***************************************************************************

function vdrCommandsOpen()
{
   vdrHaptic();
   vdrEpgView = { mode: 'commands' };

   setHtml('vdrEpgTitle', 'Befehle');
   $el('vdrEpgBack').style.visibility = 'hidden';

   let html = '';

   for (const key in commands)
      html += '<button type="button" class="vdrCommand" data-command="' + vdrEscape(key) + '"' +
         (commands[key].color ? ' style="--cmdColor:' + vdrEscape(commands[key].color) + ';"' : '') + '>' +
         vdrEscape(key) + '</button>';

   setHtml('vdrEpgRows', '<div class="vdrCommands">' + (html || '<div class="vdrEpgInfo">keine Befehle</div>') + '</div>');
   $show('vdrEpgList', true);
}

function vdrCommandExecute(key)
{
   if (!commands[key] || !vdrSocket || !vdrOnline)
      return;

   vdrHaptic();
   vdrSocket.send({ "event": "command", "object": { "command": commands[key].command } });
}
