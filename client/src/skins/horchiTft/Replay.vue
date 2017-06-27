<template>
  <div >
    <div class="replay card mt-1">
      <div v-show="replay.event.title" style="height: 100%;">
        <div class="card-block p-1" style="height: 100%;">
          <div class="progress" v-show="progress">
            <div class="progress-bar" role="progressbar"
                 :style="{width: progress + '%'}"
                 :aria-valuenow="{progress}"
                 aria-valuemin="0"
                 aria-valuemax="100">{{progress}}%
          </div>
        </div>
        <div class="row">
          <h3 class="card-title col-12 col-md-9 titletxt">{{replay.event.title}}</h3>
          <div v-if="elapsed >= 0" class="ml-auto col-12 col-md-3 desctxt">{{elapsed}}/{{parseInt(replay.lengthinseconds/60,10)}} min</div>
        </div>
        <div class="clearfix">
          <div :id="'evImages' + replay.event.eventid" class="img-fluid float-right img-thumbnail carousel slide" data-ride="carousel" data-interval="5000">
            <div class="carousel-inner epg-image" role="listbox">
              <div v-for="n in imagecnt" class="carousel-item" :class="{'active':n==1}">
                <img class="d-block" :src="'/data/recordingimg?path=' + replay.filename + '/thumbnail_0.jpg'" style="width: 100%;" alt="">
              </div>
            </div>
          </div>
          <div v-if="replay.event.epg2vdr">
            <div v-if="replay.event.epg2vdr.shorttext" class="card-text htxt">{{replay.event.shorttext}}</div>
            <div v-if="replay.event.epg2vdr.genre" class="card-text htxt">{{replay.event.epg2vdr.genre}}&nbsp;/&nbsp;{{replay.event.epg2vdr.category}}</div>
            <div v-if="replay.event.epg2vdr.episodepartname" class="card-text htxt">{{replay.event.epg2vdr.episodepartname}}</div>
            <div v-if="replay.event.epg2vdr.country" class="card-text htxt">{{replay.event.epg2vdr.country}}&nbsp;{{replay.event.epg2vdr.year}}</div>
            <div v-if="replay.event.epg2vdr.tipp" class="card-text htxt">{{replay.event.epg2vdr.tipp}}&nbsp;&nbsp;{{replay.event.epg2vdr.txtrating}}</div>
          </div>
          <div v-else="">
            <div v-if="replay.event.shorttext" class="card-text htxt">{{replay.event.shorttext}}</div>
          </div>
        <p class="desctxt" v-show="description" v-html="description"></p>
        </div>
      </div>
    </div>
    </div>
    <div class="mt-1 replay-control btn-group btn-group-sm">
      <a v-for="(button,index) in buttons" @click="$root.sendKey(button.key)" class="btn btn-secondary" :style="{'color':button.color}"><icon :name="button.icon" /></a>
    </div>
  </div>
</template>

<script>

var common = require("common");
common.Icon.register({"pause":{"width":1536,"height":1792,"paths":[{"d":"M1536 192v1408q0 26-19 45t-45 19h-512q-26 0-45-19t-19-45v-1408q0-26 19-45t45-19h512q26 0 45 19t19 45zM640 192v1408q0 26-19 45t-45 19h-512q-26 0-45-19t-19-45v-1408q0-26 19-45t45-19h512q26 0 45 19t19 45z"}]}}) ;
common.Icon.register({"play":{"width":1408,"height":1792,"paths":[{"d":"M1384 927l-1328 738q-23 13-39.5 3t-16.5-36v-1472q0-26 16.5-36t39.5 3l1328 738q23 13 23 31t-23 31z"}]}});
//common.Icon.register({"_play":{"width":1408,"height":1792,"paths":[{"d":"M242.872 0C108.732 0 0.004 108.736 0.004 242.864c0 134.14 108.728 242.876 242.868 242.876 c134.136 0 242.864-108.736 242.864-242.876C485.736 108.736 377.008 0 242.872 0z M338.412 263.94l-134.36 92.732    c-16.776 11.588-30.584 4.248-30.584-16.316V145.38c0-20.556 13.808-27.9 30.584-16.312l134.32 92.732 C355.136 233.384 355.176 252.348 338.412 263.94z"}]}})
common.Icon.register({"stop":{"width":1536,"height":1792,"paths":[{"d":"M1536 192v1408q0 26-19 45t-45 19h-1408q-26 0-45-19t-19-45v-1408q0-26 19-45t45-19h1408q26 0 45 19t19 45z"}]}})
common.Icon.register({"fast-backward":{"width":1792,"height":1792,"paths":[{"d":"M1747 141q19-19 32-13t13 32v1472q0 26-13 32t-32-13l-710-710q-9-9-13-19v710q0 26-13 32t-32-13l-710-710q-9-9-13-19v678q0 26-19 45t-45 19h-128q-26 0-45-19t-19-45v-1408q0-26 19-45t45-19h128q26 0 45 19t19 45v678q4-10 13-19l710-710q19-19 32-13t13 32v710q4-10 13-19z"}]}})
common.Icon.register({"fast-forward":{"width":1792,"height":1792,"paths":[{"d":"M45 1651q-19 19-32 13t-13-32v-1472q0-26 13-32t32 13l710 710q9 9 13 19v-710q0-26 13-32t32 13l710 710q9 9 13 19v-678q0-26 19-45t45-19h128q26 0 45 19t19 45v1408q0 26-19 45t-45 19h-128q-26 0-45-19t-19-45v-678q-4 10-13 19l-710 710q-19 19-32 13t-13-32v-710q-4 10-13 19z"}]}})

const replayButtons = [{
    key:'FastRew',
    icon:'fast-backward'
},{
    key: 'Play',
    icon: 'play'
},{
    key: 'Pause',
    icon: 'pause'
},{
    key:'Stop',
    icon:'stop'
},{
    key:'FastFwd',
    icon:'fast-forward'
}];

export default {
    name: 'o2wReplay',
    props: {
        replay: Object
    },
    data() {
        return {
            buttons: null,
            current: 0
        }
    },
    created() {
        this.$root.$on("replaycontrol", (data) => {
            this.buttons = null;
            this.current = data.current;
            if (data.active) {
                this.buttons= replayButtons;
                this.buttons[1].color = data.play == 1 ? 'red' : '#27567e'
                this.buttons[2].color = data.play != 1 ? 'red' : '#27567e'
                this.buttons[4].color = data.speed >= 0 && data.forward == 1 ? 'red' : '#27567e'
                this.buttons[0].color = data.speed >= 0 && data.forward != 1 ? 'red' : '#27567e'
            }
        });
    },
    computed: {
        description: function () {
            return this.replay.event.epg2vdr && this.replay.event.epg2vdr.longdescription ?
                this.replay.event.epg2vdr.longdescription.replace(/\n/g, '<br />') :
                this.replay.event.epg2vdr && this.replay.event.epg2vdr.shortdescription ?
                this.replay.event.epg2vdr.shortdescription.replace(/\n/g, '<br />') :
                this.replay.event.description ? this.replay.event.description.replace(/\n/g, '<br />') : '';
        },
        progress: function () {
            if (this.current >= 0) {
                window.setTimeout(() => {
                    this.current += 10;
                }, 10000);
            }
            return Math.max(parseInt(this.current / this.replay.lengthinseconds * 100, 10), 1);
        },
        elapsed: function () {
            return Math.max(parseInt(this.current/60, 10), 0);
        },
        imagecnt: function() {
            let cnt = this.replay.event.epg2vdr ? parseInt(this.replay.event.epg2vdr.imagecount, 10) : 0;
            // return isNaN(cnt) ? 0 : cnt;
            return 1;
        }
    }
}
</script>
