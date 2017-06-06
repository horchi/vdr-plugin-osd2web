var common= require("common");
import WebSocketClient from "./websocket.js"
common.Icon.register({"skinfocus":{"width":2048,"height":1792,"paths":[{"d":"M1792 1248v-960q0-13-9.5-22.5t-22.5-9.5h-1600q-13 0-22.5 9.5t-9.5 22.5v960q0 13 9.5 22.5t22.5 9.5h1600q13 0 22.5-9.5t9.5-22.5zM1920 288v960q0 66-47 113t-113 47h-736v128h352q14 0 23 9t9 23v64q0 14-9 23t-23 9h-832q-14 0-23-9t-9-23v-64q0-14 9-23t23-9h352v-128h-736q-66 0-113-47t-47-113v-960q0-66 47-113t113-47h1600q66 0 113 47t47 113z"}]}})

export var Vue= common.Vue;

// Registrieren aller Komponenten:
Vue.component('o2w-event', require('Event'))
Vue.component('o2w-textmenu', require('Textmenu'))
Vue.component('o2w-textarea', require('Textarea'))
Vue.component('o2w-actual', require('Actual'))
Vue.component('o2w-timer', require('Timer'))
Vue.component('o2w-navigation', require('Navigation'))
Vue.component('o2w-remote', require('Remote'))
Vue.component('o2w-osd', require('Osd'))
Vue.component('o2w-statusmessage', require('Statusmessage'))
Vue.component('o2w-overview', require('Overview'))
Vue.component('o2w-replaycontrol', require('ReplayControl'))

export var root = {
    data: {
        isOnlyView: /[?&]onlyView/.test(location.search),   // reine Anzeige ohne direkte Interaktion über den Client
        isActive: false,                                    // ist der Client am VDR als aktive angemeldet
        skinAttached: false,
        hasChannelLogos: false,
        osdOn: false,
        menuItems: [],                                      // 
        menuItemsRight: [],
        keyMap: {
            38: 'Up',
            40: 'Down',
            13: 'Ok',
            8: 'Back',
            27: 'Back',
            37: 'Left',
            39: 'Right',
            48: '0',
            49: '1',
            50: '2',
            51: '3',
            52: '4',
            53: '5',
            54: '6',
            55: '7',
            56: '8',
            57: '9'
            /*
            Up
            Down
            Menu
            Ok
            Back
            Left
            Right
            Red
            Green
            Yellow
            Blue
            0
            1
            2
            3
            4
            5
            6
            7
            8
            9
            Info
            Play/Pause
            Play
            Pause
            Stop
            Record
            FastFwd
            FastRew
            Next
            Prev
            Power
            Channel+
            Channel-
            PrevChannel
            Volume+
            Volume-
            Mute
            Audio
            Subtitles
            Schedule
            Channels
            Timers
            Recordings
            Setup
            Commands
            User0
            User1
            User2
            User3
            User4
            User5
            User6
            User7
            User8
            User9
            _Setup
                */
        }
    },
    render: h => h(require('App')),
    methods: {
        sendKey(key, repeat) {
            this.$socket.send({
                event: "keypress",
                object: {
                    "key": key,
                    "repeat": (repeat || 1)
                }
            });
        },
        mapKey(keyString) {
            let key = this.keyMap[keyString];
            if (key) this.sendKey(key);
        },
        formatDateTime(unixTime) {
            var d = new Date(unixTime * 1000); // - this.timeOffset
            return d.toLocaleDateString('de-DE', {
                weekday: 'short',
                year: 'numeric',
                month: 'long',
                day: 'numeric'
            }) + ' ' + new String(100 + d.getHours()).slice(1) + ':' + new String(100 + d.getMinutes()).slice(1);
        },
        formatTime(unixTime) {
            var d = new Date(unixTime * 1000); // - this.timeOffset
            return new String(100 + d.getHours()).slice(1) + ':' + new String(100 + d.getMinutes()).slice(1);
        }
        /*,
                formatDate(unixTime) {
                    var d = new Date(unixTime * 1000); // - this.timeOffset
                    return d.toLocaleDateString('de-DE', { weekday: 'short', year: 'numeric', month: 'long', day: 'numeric' });
                }*/

    },
    created() {
        try {
            this.$socket = new WebSocketClient({
                url: "ws://" + location.host,
                protocol: "osd2vdr",
                autoReconnectInterval: 10000,
                onopen: () => {
                    if ( this.isActive === null) // Dann wurde es beim Schliessen auf null gesetzt
                        this.$socket.send({ "event": "login", "object": { "type": + (this.isOnlyView ? 1 : 0) } });
                }, onclose: () => {
                    this.isActive = null; // auf null setzten, dass ein neues login aufgerufen wird
                },
                onmessage: (msg) => {
                    try {
                        let data = JSON.parse(msg.data.replace());
                        this.$emit(data.event, data.object);
                    } catch (e) {
                        console.log(e);
                    }
                }
            });
            if (!this.$socket)
                return !(this.$el.innerHTML = "Your Browser will not support Websockets!");

            this.$socket.send({ "event": "login", "object": { "type": + (this.isOnlyView ? 1 : 0) } });

            // Nachrichten/Anfragen der Komponenten an den Server weiterleiten
            this.$on("send", this.$socket.send);

            // Globale Nachrichten verarbeiten
            this.$on("rolechange", (data) => {
                this.isActive = data.role == 'active';
                this.hasChannelLogos = data.havelogos == 1;
            })

            // Interne Nachrichten verarbeiten
            this.$on("osdState", (data) => {
                this.osdOn= data.active;
                //document.getElementById('o2wContent').style.display= data.active ? "none" : '';
            });

            // Status des VDR-Skins
            let skinMenuItem = {
                label: '',
                on: false,
                icon: 'skinfocus',
                func: function (navComp) {
                    navComp.$root.$emit("send", {
                        "event": (this.on ? "leavefocus" : "takefocus")
                    });
                }
            };
            this.$on("skinstate", (data) => {
                this.skinAttached = data.attached == 1;
                this.$root.$set(skinMenuItem, "on", this.skinAttached);
            })
            this.menuItemsRight.push(skinMenuItem);


            if (this.isOnlyView) {
                var scrollingElement = document.scrollingElement || document.documentElement;
                window.autoScroll = function (delta) {

                    let hasVScroll = document.body.scrollHeight > document.body.clientHeight;
                    let cStyle = document.body.currentStyle || window.getComputedStyle(document.body, "");
                    hasVScroll = cStyle.overflow == "visible"
                        || cStyle.overflowY == "visible"
                        || (hasVScroll && cStyle.overflow == "auto")
                        || (hasVScroll && cStyle.overflowY == "auto");

                    let nextCall = 3000;
                    if (hasVScroll) {
                        let curTop = scrollingElement.scrollTop;
                        scrollingElement.scrollTop += delta;
                        if (curTop == scrollingElement.scrollTop)
                            delta *= -1;
                        nextCall = 80;
                    }
                    window.setTimeout(window.autoScroll, nextCall, delta);
                }
                window.autoScroll(1);
            } else {
                // Browserevent abfangen
                window.addEventListener('keyup', (ev) => {
                    this.mapKey((ev.altKey ? 'alt.' : '') + (ev.ctrlKey ? 'ctrl.' : '') + (ev.shiftKey ? 'shift.' : '') + ev.keyCode)
                });
            }

            window.addEventListener("unload", () => {
                this.$socket.send({ "event": "logout", "object": {} });
            })

        } catch (exception) {
            alert('<p>Error' + exception);
        }
    }
}

/* TODO
    { "event" : "channels" } holt alle channels -> sollte ggf. gecacht werden

*/


/*
enum ObjectShape
     {
        osText   = 0x01,
        osSmall  = 0x02,
        osLarge  = 0x04,
     };

mögliche langeobjekte:
     5 -> Kanäle
     3 -> Programm 'Jetzt'
     4 -> Programm 'Next'
     2 -> Programm
     7 -> Timer
     9 -> Aufzeichnungen

*/
