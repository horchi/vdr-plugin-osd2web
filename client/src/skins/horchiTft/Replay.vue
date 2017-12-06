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
        <div class="eventtitlerow clearfix">
          <div class="titletxt">{{replay.event.title}}</div>
          <div v-if="elapsed >= 0" class="durationtxt">{{remaining}}/{{parseInt(replay.lengthinseconds/60,10)}}</div>
        </div>
        <div class="clearfix">
          <div :id="'evImages' + replay.event.eventid" class="img-fluid float-right img-thumbnail carousel slide" data-ride="carousel" data-interval="5000">
            <div class="carousel-inner epg-image" role="listbox">
              <div v-for="(img, n) in replay.images" class="carousel-item" :class="{'active':n==0}">
                <img class="d-block" :src="'/data/recordingimg?path=' + img" style="width: 100%;" alt="">
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
    <div class="mt-1 replay-control">
      <a v-for="(button,index) in buttons" @click="$root.sendKey(button.key)"><icon :class="button.cls" :name="button.icon" /></a>
    </div>
  </div>
</template>

<script>

var common = require("common");
common.Icon.register({"pause":{"width":1536,"height":1792,"paths":[{"d":"M1536 192v1408q0 26-19 45t-45 19h-512q-26 0-45-19t-19-45v-1408q0-26 19-45t45-19h512q26 0 45 19t19 45zM640 192v1408q0 26-19 45t-45 19h-512q-26 0-45-19t-19-45v-1408q0-26 19-45t45-19h512q26 0 45 19t19 45z"}]}}) ;
common.Icon.register({"play":{"width":1408,"height":1792,"paths":[{"d":"M1384 927l-1328 738q-23 13-39.5 3t-16.5-36v-1472q0-26 16.5-36t39.5 3l1328 738q23 13 23 31t-23 31z"}]}});
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
            current: 0,
            isPlaying: false
        }
    },
    created() {
        this.$root.$on("replaycontrol", (data) => {
            this.buttons = null;
            this.current = data.current;
            this.isPlaying= data.play == 1;
            if (data.active) {
                this.buttons = replayButtons;
                this.buttons[0].cls = data.speed >= 0 && data.forward != 1 ? 'replay-btn replay-btn-act' : 'replay-btn'
                this.buttons[1].cls = this.isPlaying ? 'replay-btn replay-btn-act' : 'replay-btn'
                this.buttons[2].cls = this.isPlaying ? 'replay-btn' : 'replay-btn replay-btn-act'
                this.buttons[3].cls = 'replay-btn'
                this.buttons[4].cls = data.speed >= 0 && data.forward == 1 ? 'replay-btn replay-btn-act' : 'replay-btn'
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
                    if (this.isPlaying)
                        this.current += 10;
                }, 10000);
            }
            return Math.max(parseInt(this.current / this.replay.lengthinseconds * 100, 10), 1);
        },
        remaining: function () {
            return Math.max(parseInt((this.replay.lengthinseconds - this.current)/60, 10), 0);
        },
        elapsed: function () {
            return Math.max(parseInt(this.current/60, 10), 0);
        }
    },
    updated: function() {
        $('.carousel').carousel().each(function(){ $('.carousel-item',this).removeClass('active').first().addClass('active')});
    }
}
</script>
