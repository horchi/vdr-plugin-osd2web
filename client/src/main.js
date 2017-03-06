import Vue from 'vue'
import App from './App.vue'

import "../vuikit-theme/dist/css/uikit.css"

window.v = new Vue({
    data: {
        menuItems: [],
        menuItemsRight: [{
           label: '',
           on:false,
           iconClass: 'uk-icon-desktop',
           func: function(navComp){
              this.on= !this.on;
              navComp.$root.$emit("send",{"event": (this.on ? "takefocus" : "leavefocus")});
           }
       }]
    },
    render: h => h(App),
    methods: {
        sendKey(key, repeat) {
            this.$socket.send(JSON.stringify({
                event: "keypress",
                object: {
                    "key": key,
                    "repeat": (repeat || 1)
                }
            }));
        },
        mapKey(keyString) {
            let key = this.$keyMap[keyString];
            if (key)
                this.sendKey(key);
        }
    },
    created() {

        if (!window.WebSocket)
            return !(this.$el.innerHTML = "Your Browser will not support Websockets!");
        this.$socket = new WebSocket("ws://" + location.host, "osd2vdr");

        try {
            this.$socket.onopen = function() {
                console.log("websocket connection opened");
            }

            this.$socket.onmessage = (msg) => {
                try {
                    let data = JSON.parse(msg.data.replace());
                    this.$emit(data.event, data.object);
                } catch (e) {
                    console.log(e);
                }
            }

            this.$socket.onclose = function() {
                console.log("websocket connection closed");
            }

            this.$socket.onerror = function(err) {
                console.log("websocket error: ", err);
            }

            this.$on("send", (JSONobj) => {
                this.$socket.send(JSON.stringify(JSONobj));
            })
            this.$keyMap = {
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
            }

            window.addEventListener('keyup', (ev) => {
                this.mapKey((ev.altKey ? 'alt.' : '') + (ev.ctrlKey ? 'ctrl.' : '') + (ev.shiftKey ? 'shift.' : '') + ev.keyCode)
            });

            /*  window.addEventListener("unload", () =>{
this.$socket.send({"action":"close"});
})
Alle 60 sekunden ein ping senden, damit die connection nicht geschlossen wird
//, bzw. mitzubekommen, wenn der Server wieder da ist
window.setTimeout(() =>{
this.$socket.send({"action":"ping"});
},60000);    */
        } catch (exception) {
            alert('<p>Error' + exception);
        }
    }
}).$mount('#app')
