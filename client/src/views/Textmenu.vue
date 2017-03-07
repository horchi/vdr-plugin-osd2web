<template>
<div v-show="title" class="uk-overflow-container uk-flex-space-between uk-width-1" :class="{'uk-text-nowrap':$root.onlyRemote}" id="osdCon">
    <div class="uk-panel uk-panel-hover" @click="sendKey('Back')">
        <h3 class="uk-panel-title"><i class="uk-icon-mail-reply-all"></i> {{ title }}</h3>
    </div>
    <table v-show="colCount" class="uk-table uk-table-hover">
        <tbody>
            <tr v-for="(row, rowIndex) in rows" :key="rowIndex" :class="{'uk-form-success': rowIndex == textmenucurrent}" @click="row.selectable && doAction(rowIndex)">
                <td v-for="(col, index) in row.cols" :key="index" :class="{'uk-placeholder': rowIndex == textmenucurrent && index== 1 && canEdit == 1}">{{col}}</td>
                <td v-if="row.selectable && canEdit==1" class="uk-button-group">
                    <a class="uk-button" @click.stop="doAction(rowIndex,'Left')"><i class="uk-icon-caret-square-o-left"></i></a>
                    <a v-show="row.textEdit" class="uk-button" @click.stop="sendKey('Up')"><i class="uk-icon-caret-square-o-up"></i></a>
                    <a v-show="row.textEdit" class="uk-button" @click.stop="sendKey('Down')"><i class="uk-icon-caret-square-o-down"></i></a>
                    <a class="uk-button" @click.stop="doAction(rowIndex,'Right')"><i class="uk-icon-caret-square-o-right"></i></a>
                </td>
            </tr>
        </tbody>
    </table>
    <div v-show="event.title" class="uk-panel">
        <h3 class="uk-panel-title">{{event.title}}</h3>
        <div class="uk-panel-teaser">{{event.shorttext}}</div>
        <div class="uk-progress">
            <div class="uk-progress-bar" :style="event.progress"></div>
        </div>
        <div class="uk-flex uk-flex-space-between">
          <div class="uk-width-2-3 uk-panel">
             {{formatDateTime(event.starttime)}}&nbsp;-&nbsp;{{formatTime(event.endtime)}}
          </div>
          <div class="uk-width-1-3 uk-panel uk-float-right">
             {{event.duration/60}} min</div>
        </div>

    </div>
    <div v-show="text" class="uk-panel-box uk-grid-margin" v-html="text"></div>
    <div class="uk-button-group" id="buttons" style="position:fixed;bottom:0">
        <button v-for="(button,index) in buttons" @click="sendKey(button.color)" :class="'but-' + button.color" class="uk-button uk-margin-small-top" type="button">{{button.label}}</button>
    </div>
</div>
</template>

<script>
function getClearData(){
  return {
      title: '',
      rows: [],
      colCount: 0,
      textmenucurrent: 0,
      canEdit: 0,
      category: -1,
      text: '',
      event:{},
      buttons: []
  }
}
export default {
    name: 'o2vOsd',
    data: function() {
        return getClearData();
    },
    created() {
        this.$root.$data.menuItems.push({
            label: '_O_SD',
            key: 'menu'
        });
        this.$root.$on("clearmenu", (data) => {
           let clearData= getClearData();
           for (let key in clearData)
            this[key]= clearData[key];
        });
        this.$root.$on("menu", (data) => {
            this.category = data.category;
            this.canEdit = data.editable || 0;
            this.title = data.title;
        });
        this.$root.$on("menuitem", (data) => {
            if (!this.colCount) {
                this.colCount = data.text.split('\t').length;
                if (this.colCount == 2 && this.canEdit)
                    this.colCount++;
                else
                    this.canEdit = 0;
            }
            this.$set(this.rows, data.index, {
                cols: data.text.split('\t'),
                selectable: data.selectable,
                textEdit: this.canEdit && /.+\\t.*\[.\]/.test(data.text)
            });
            if (data.current)
                this.textmenucurrent = data.index;
        });
        this.$root.$on("menutext", (data) => {
            this.text = data.text.replace(/\n/g, '<br />');
        });
        this.$root.$on("textscroll", (data) => {
             if (data.page) {
                 if (data.up)
                     document.body.scrollLeft = Math.max(0, document.body.scrollLeft - document.body.clientWidth);
                 else
                     document.body.scrollLeft += document.body.clientWidth;
             } else {
                 if (data.up)
                     document.body.scrollTop = Math.max(0, document.body.scrollTop - document.body.clientHeight);
                 else
                     document.body.scrollTop = document.body.scrollTop + document.body.clientHeight;
             }
        });
        this.$root.$on("event", (data) => {
            this.event = data;
            this.text = data.description.replace(/\n/g, '<br />');
            this.event.progress= 'width:' + ((parseInt(new Date().getTime() / 1000, 10) - data.starttime) / data.duration * 100) + '%';
        });
        this.$root.$on("buttons", (data) => {
            this.buttons = [];
            for (let color in data)
                this.buttons.push({
                    'color': color,
                    label: data[color]
                });
        })
        window.addEventListener('resize', this.checkButtonHeight);
    },
    updated() {
        this.checkButtonHeight();
    },
    methods: {
        sendKey(key) {
            this.$root.sendKey(key);
        },
        doAction(rowIndex, key) {
            let delta = rowIndex - this.textmenucurrent;
            if (delta != 0)
                this.$root.sendKey(delta > 0 ? "Down" : "Up", Math.abs(delta));
            else
                this.$root.sendKey(key || "Ok");
        },
        checkButtonHeight() {
            let buttons = document.getElementById('buttons');
            if (buttons)
                buttons.parentNode.style.paddingBottom = buttons.offsetHeight + 'px';
        },
        formatDateTime(unixTime) {
           var d = new Date(unixTime * 1000); // - this.timeOffset
           return d.toLocaleDateString('de-DE', { weekday: 'short', year: 'numeric', month: 'long', day: 'numeric' })
               + ' ' + new String(100 + d.getHours()).slice(1) + ':' + new String(100 + d.getMinutes()).slice(1);
        },
        formatTime(unixTime) {
            var d = new Date(unixTime * 1000); // - this.timeOffset
            return new String(100 + d.getHours()).slice(1) + ':' + new String(100 + d.getMinutes()).slice(1);
        }/*,
        formatDate(unixTime) {
            var d = new Date(unixTime * 1000); // - this.timeOffset
            return d.toLocaleDateString('de-DE', { weekday: 'short', year: 'numeric', month: 'long', day: 'numeric' });
        }*/
    }
}
//      const char *charMap = tr("CharMap$ 0\t-.,1#~\\^$[]|()*+?{}/:%@&\tabc2\tdef3\tghi4\tjkl5\tmno6\tpqrs7\ttuv8\twxyz9");
//       msgstr " 0\t-.,1#~\\^$[]|()*+?{}/:%@&\tabcä2\tdef3\tghi4\tjkl5\tmnoö6\tpqrsß7\ttuvü8\twxyz9"


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
</script>

<style>
button.but-red {
    background-color: red;
    color: #fff;
}




button.but-green {
    background-color: green;
    color: #fff;
}




button.but-blue {
    background-color: blue;
    color: #fff;
}




button.but-yellow {
    background-color: yellow;
    color: #222;
}
</style>
