<template>
<div v-if="colCount > 0" class="uk-overflow-container">
   <table class="uk-table uk-table-hover">
        <thead>
            <tr>
                <th :colspan="colCount"><a class="uk-button" @click="sendKey('Back')"><i class="uk-icon-mail-reply-all"></i> {{ title }}</a></th>
            </tr>
        </thead>
        <tbody>
            <tr v-for="(row, index) in rows" :key="index" :class="{'uk-active': index == textmenucurrent}" v-on:click="row.selectable && doAction(index)">
                <td v-for="(col, index) in row.cols" :key="index">{{col}}</td>
            </tr>
        </tbody>
    </table>
    <div class="uk-button-group" id="buttons" style="position:fixed;bottom:0">
      <button v-for="(button,index) in buttons"
         @click="sendKey(button.color)"
         :class="'but-' + button.color" class="uk-button uk-margin-small-top"
         type="button">{{button.label}}</button>
   </div>
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
        this.$root.$on("clearmenu", (data) => {
           this.colCount = 0;
           this.category = null;
           this.rows = [];
        });
        this.$root.$on("menu", (data) => {
           this.category = data.category;
           this.title = data.title;
        });
        this.$root.$on("menuitem", (data) => {
           if (!this.colCount)
               this.colCount= data.text.split('\t').length;
           this.$set(this.rows, data.index, {cols:data.text.split('\t'), selectable: data.selectable});
           if (data.current)
               this.textmenucurrent = data.index;
        });
        this.$root.$on("buttons", (data) => {
            this.buttons = [];
            for (let color in data)
               this.buttons.push({'color':color, label:data[color]});
        })
        window.addEventListener('resize', this.checkButtonHeight);
    },
    updated(){
      this.checkButtonHeight();
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
        },
        checkButtonHeight(){
           let buttons= document.getElementById('buttons');
           if (buttons)
               buttons.parentNode.style.paddingBottom= buttons.offsetHeight + 'px';
        }
    }
}
//      const char *charMap = tr("CharMap$ 0\t-.,1#~\\^$[]|()*+?{}/:%@&\tabc2\tdef3\tghi4\tjkl5\tmno6\tpqrs7\ttuv8\twxyz9");
//       msgstr " 0\t-.,1#~\\^$[]|()*+?{}/:%@&\tabcä2\tdef3\tghi4\tjkl5\tmnoö6\tpqrsß7\ttuvü8\twxyz9" 

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
