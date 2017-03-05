<template>
<div v-if="colCount > 0" class="uk-overflow-container">
   <div class="uk-button-group">
      <button v-for="(button,index) in buttons" @click="sendKey(button.color)" :class="'but-' + button.color" class="uk-button uk-margin-small-top" >{{button.label}}</button>
   </div>
   <table class="uk-table uk-table-hover">
        <thead>
            <tr>
                <th :colspan="colCount"><a class="uk-button" @click="sendKey('Back')"><i class="uk-icon-mail-reply-all"></i> {{ title }}</a></th>
            </tr>
        </thead>
        <tbody>
            <tr v-for="(row, index) in rows" :key="index" :class="{'uk-active': index == textmenucurrent}" v-on:click="doAction(index)">
                <td v-for="(col, index) in row" :key="index">{{col}}</td>
            </tr>
        </tbody>
    </table>
</div>

</template>

<script>
export default {
    name: 'o2vOsd',
    data: function() {
        return {
            title: '',
            rows: [],
            colCount: 0,
            textmenucurrent: 0,
            category: -1,
            buttons:[]
        };
    },
    created() {
        this.$root.$data.menuItems.push({
            label: '_O_SD',
            key:  'menu'
        });
        this.$root.$on("textmenu", (data) => {
            if (!data.rows.length && data.rowcount > 0)
                return; // Falsche Daten
            this.title = data.title;
            this.colCount = parseInt(data.colcount || 0, 10) + 1;
            this.category = data.category;
            this.rows = [];
            for (let i = 0; i < data.rows.length; i++) {
                this.rows[i] = data.rows[i].split('\t');
            }
        })
        this.$root.$on("textmenucurrent", (data) => {
            if (this.category == data.category){
                this.$set(this.rows, data.index, data.row.split('\t'));
                this.textmenucurrent = data.index;
            }
        })
        this.$root.$on("closemenu", (data) => {
            this.colCount = this.textmenucurrent = 0;
        })
        this.$root.$on("colorbuttons", (data) => {
            this.buttons = [];
            for (let color in data)
               this.buttons.push({'color':color, label:data[color]});
        })
    },
    methods: {
        sendKey(key) {
            this.$root.sendKey(key);
        },
        doAction(rowIndex) {
            let delta = rowIndex - this.textmenucurrent;
            if (delta != 0)
               this.$root.sendKey(delta > 0 ? "Down" : "Up", Math.abs(delta));
               else
            this.$root.sendKey("Ok");
        }
    }
}

/*
enum eMenuCategory {
   mcUndefined = -1,
   mcUnknown = 0,
   mcMain,
   mcSchedule,
   mcScheduleNow,
   mcScheduleNext,
   mcChannel,
   mcChannelEdit,
   mcTimer,
   mcTimerEdit,
   mcRecording,
   mcRecordingInfo,
   mcRecordingEdit,
   mcPlugin,
   mcPluginSetup,
   mcSetup,
   mcSetupOsd,
   mcSetupEpg,
   mcSetupDvb,
   mcSetupLnb,
   mcSetupCam,
   mcSetupRecord,
   mcSetupReplay,
   mcSetupMisc,
   mcSetupPlugins,
   mcCommand,
   mcEvent,
   mcText,
   mcFolder,
   mcCam
   };

*/

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
      background-color:red;
      color:#fff;
   }
   button.but-green {
       background-color: green;
       color:#fff;
   }
   button.but-blue {
      background-color:blue;
      color:#fff;
   }
   button.but-yellow {
       background-color: yellow;
       color:#222;
   }
</style>
