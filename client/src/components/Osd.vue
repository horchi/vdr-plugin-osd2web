<template>
  <div class="row dataarea">
    <div id="eventarea" class="eventarea col-12 col-md-9">
      <div class="menutitle" id="osdTitle" @click="$root.sendKey('Back')">
          <icon v-if="!$root.isOnlyView" name="osd-back"></icon>
          {{ title }}
      </div>
      <o2w-eventmenu v-show="object_menu == true" />
      <o2w-textmenu v-show="object_menu != true" />
      <o2w-event v-show="have_event" :event="event" />
      <o2w-textarea />
      <o2w-osdbuttons />
    </div>
    <div class="rightarea pr-2 col-12 col-md-3">
      <div v-if="object_menu">
        <o2w-rightevent :event="eventitem" />
      </div>
      <div v-else="">
        <o2w-timer class="timerarea" />
        <o2w-customdata class="customarea" />
        <o2w-recording class="recordingarea" />
      </div>
    </div>
    <o2w-statusbar />
  </div>
</template>

<script>

var common=require("common");
common.Icon.register({"osd-back":{"width":1280,"height":1792,"paths":[{"d":"M1171 301l-531 531 531 531q19 19 19 45t-19 45l-166 166q-19 19-45 19t-45-19l-742-742q-19-19-19-45t19-45l742-742q19-19 45-19t45 19l166 166q19 19 19 45t-19 45z"}]}})

// var maxLines= 0;

function getClearData() {
  return {
      title: '',
      category: -1,
      event: {},
      eventitem: {},
      have_event: false,
      object_menu: null
      // maxLines: maxLines   // maximale Anzahl Zeilen, die der Client darstellen kann
  }
}
export default {
    name: 'o2wOsd',
    data: function() {
        return getClearData();
    },
    created() {
        let menuItem= {
            label: '_O_SD',
            isHidden: function(navComp) {
                return !navComp.$root.isActive
            },
            key: 'menu'
        };
        this.$root.$data.menuItems.push(menuItem);
        this.$root.$on("clearmenu", (data) => {
            let clearData= getClearData();
            for (let key in clearData)
                this[key]= clearData[key];
            this.$root.$set(menuItem, "on", false);
        });
        this.$root.$on("menu", (data) => {
            this.category = data.category;
            this.title = data.title;
            this.$root.$set(menuItem, "on", true);
        });
        this.$root.$on("event", (data) => {
            this.have_event = true;
            this.event = data;
        });
        this.$root.$on("eventitem", (data) => {
            this.object_menu = true;
            if (data.current)
                this.eventitem = data.event;
        });
        this.$root.$on("rolechange", (data) => {
            this.sendMaxLines();
        });

        if (!common.maxLinesCalc.canScroll) {
            let lastResize= 0;
            let checkInt= false;
            let _this= this;
            function checkResize(ev) {
                if (!checkInt) {
                    checkInt= window.setInterval(checkResize,500);
                } else {
                    if ((new Date().getTime() - lastResize) > 500 ) {
                        _this.sendMaxLines();
                        checkInt= !!window.clearInterval(checkInt);
                    }
                }
                lastResize= new Date().getTime();
            }
            window.addEventListener('resize', checkResize);
        }

        //window.addEventListener('resize', this.checkButtonHeight);
    },
    updated() {
        if (this.title) {
            this.$root.$emit("curView", "osd");
            if (!common.maxLinesCalc.canScroll) {
                window.setTimeout(this.sendMaxLines, 500); // bisschen zeitverzögert, damit auch die Buttons und Zeilen geschrieben sind
            }
        } else {
            this.$root.$emit("curView", null);
        }
        //this.checkButtonHeight();
    },
    methods: {
        sendMaxLines() {
            //let max = common.maxLinesCalc.getMax();
            // if (max != maxLines) {
            //            console.log(common.maxLinesCalc)
            //maxLines = max;
            let data = [];
            data.push({
                "category": this.category,
                "maxlines": common.maxLinesCalc.canScroll ? 100 : eMenuCategory[this.category].maxlines,
                "shape": common.maxLinesCalc.canScroll ? 1 : eMenuCategory[this.category].shape
            });
            //for (let i = 0; i < eMenuCategory.length; i++) data.push({
            //    "category": i,
            //    "maxlines": common.maxLinesCalc.canScroll ? 100 : eMenuCategory[i].maxlines,  // max
            //    "shape": eMenuCategory[i].shape
            //});
            this.$root.$emit("send", {
                "event": "maxlines",
                object: {
                    "categories": data
                }
            });
            //  }
        }/*,
        checkButtonHeight() {
            let buttons = document.getElementById('buttons');
            if (buttons)
                buttons.parentNode.style.paddingBottom = buttons.offsetHeight + 'px';
        }*/
    }
}

const eMenuCategory = [
    {
        "category": 'mcUnknown',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcMain',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcSchedule',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcScheduleNow',
        "maxlines": 10,
        "shape": 2
    }, {
        "category": 'mcScheduleNext',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcChannel',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcChannelEdit',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcTimer',
        "maxlines": 10,
        "shape": 1 //4 für ausführliche Informationen
    }, {
        "category": 'mcTimerEdit',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcRecording',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcRecordingInfo',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcRecordingEdit',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcPlugin',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcPluginSetup',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcSetup',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcSetupOsd',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcSetupEpg',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcSetupDvb',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcSetupLnb',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcSetupCam',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcSetupRecord',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcSetupReplay',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcSetupMisc',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcSetupPlugins',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcCommand',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcEvent',
        "maxlines": 10,
        "shape": 1 //4 für ausführliche Informationen
    }, {
        "category": 'mcText',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcFolder',
        "maxlines": 10,
        "shape": 1
    }, {
        "category": 'mcCam',
        "maxlines": 10,
        "shape": 1
    }
];
eMenuCategory['-1'] = {
    "category": 'mcUndefined',
    "maxlines": 10,
    "shape": 1
};

</script>
