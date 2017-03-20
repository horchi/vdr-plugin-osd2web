import Vue from 'vue'
import App from './App.vue'
import "uikit/dist/css/uikit.min.css"

window.v = new Vue({
    data: {
        isOnlyView: /[?&]onlyView/.test(location.search),
        isActive: false,
        maxLines: 0,
        osdOn: false,
        skinAttached: false,
        hasChannelLogos: false,
        menuItems: [],
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
    render: h => h(App),
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
        sendMaxLines(ev, maxLines) { //  header - buttons
            let max = maxLines || parseInt((window.innerHeight - 128 - 40) / 38, 10)
            if (max != this.maxLines) {
                this.maxLines = max;
                let data = [];
                for (let i = 0; i < eMenuCategory.length; i++) data.push({
                    "category": i,
                    "maxlines": max,
                    "shape": eMenuCategory[i].shape
                });
                this.$emit("send", {
                    "event": "maxlines",
                    object: {
                        "categories": data
                    }
                });
            }
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
                    console.log("websocket connection opened");
                    this.$socket.send({ "event" : "login", "object" : { "type" : + (this.isOnlyView ? 1 : 0) } });
                    this.sendMaxLines(null);
                },
                onclose:() => {
                    this.isActive = false;
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
            //        UIkit.notification({message: 'Copied!', pos: 'bottom-right'}); 



            // Nachrichten/Anfragen der Module an den Server weiterleiten
            this.$on("send", this.$socket.send);

            // Browserevent abfangen
            !this.isOnlyView && window.addEventListener('keyup', (ev) => {
                this.mapKey((ev.altKey ? 'alt.' : '') + (ev.ctrlKey ? 'ctrl.' : '') + (ev.shiftKey ? 'shift.' : '') + ev.keyCode)
            });
            window.addEventListener("unload", () => {
                this.$socket.send({ "event" : "logout", "object" : {} });
            })
            window.addEventListener('resize', this.sendMaxLines);

            // Globale Nachrichten verarbeiten
            this.$on("rolechange", (data) => {
                this.isActive = data.role == 'active';
                this.hasChannelLogos = data.havelogos == 1;
            })
            let skinMenuItem= {
                label: '',
                on: false,
                icon: 'tv',
                func: function (navComp) {
                    this.on = this.on;
                    navComp.$root.$emit("send", {
                        "event": (this.on ? "takefocus" : "leavefocus")
                    });
                }
            };
            this.$on("skinstate", (data) => {
                this.skinAttached = data.attached == 1;
                this.$root.$set(skinMenuItem, "on",this.skinAttached);
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
            }
        } catch (exception) {
            alert('<p>Error' + exception);
        }
    }
}).$mount('#app')



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
const eMenuCategory = [
    {
        "category": 'mcUnknown',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcMain',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcSchedule',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcScheduleNow',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcScheduleNext',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcChannel',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcChannelEdit',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcTimer',
        "maxlines": 100,
        "shape": 4
    }, {
        "category": 'mcTimerEdit',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcRecording',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcRecordingInfo',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcRecordingEdit',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcPlugin',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcPluginSetup',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcSetup',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcSetupOsd',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcSetupEpg',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcSetupDvb',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcSetupLnb',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcSetupCam',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcSetupRecord',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcSetupReplay',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcSetupMisc',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcSetupPlugins',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcCommand',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcEvent',
        "maxlines": 100,
        "shape": 4
    }, {
        "category": 'mcText',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcFolder',
        "maxlines": 100,
        "shape": 1
    }, {
        "category": 'mcCam',
        "maxlines": 100,
        "shape": 1
    }
];
eMenuCategory['-1'] = {
    "category": 'mcUndefined',
    "maxlines": 100,
    "shape": 1
};

/* WebsocketClient, usage:
wsClient = new WebSocketClient({
    url: "ws://" + location.host,
    protocol: "optional",
    autoReconnectInterval: 5000, // default is 0, without reconnect
    onopen: () => {  ...  },
    onmessage: (msg) => {  ...  },
    onclose: function () {  ...  },
    onerror: function (err) {  ...  }
});
*/
function WebSocketClient(opt) {
    if (!window.WebSocket) return false;

    let client= this;
    this.onclose= function () {
        console.log("websocket connection closed");
    }
    this.onerror= function (err) {
        console.log("websocket error: ", err);
    }
    for (var key in opt)
        this[key]= opt[key];
    this.reconnect= function(e){
        if (client.autoReconnectInterval){
            console.log(`WebSocketClient: retry in ${client.autoReconnectInterval}ms`, e);
            setTimeout(function () {
                console.log("WebSocketClient: reconnecting...");
                client.open();
            }, client.autoReconnectInterval);
        }
    }
    this.send= function(JSONobj){
        client.ws.send(JSON.stringify(JSONobj));
    }
    this.open= function(){
        client.ws = new WebSocket(client.url, client.protocol);
        client.ws.onopen = client.onopen;
        client.ws.onmessage = client.onmessage;
        client.ws.onclose= function(e) {
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
        client.ws.onerror = function(e){
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
    this.reopen= function(){
        client.ws.onerror= client.ws.onclose= client.ws.onmessage= null;
        client.ws.close();
        this.open();
    }
    this.open();
    return this;
}
