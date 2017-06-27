<template>
  <div v-else="" class="replay card mt-1">
    <div v-show="replay.title" style="height: 100%;">
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
          <h3 class="card-title col-12 col-md-10 titletxt">{{replay.title}}</h3>
          <div v-if="elapsed > 0" class="ml-auto col-12 col-md-2 desctxt">{{elapsed}}/{{parseInt(replay.duration/60,10)}} min</div>
          <div v-if="elapsed <= 0" class="ml-auto col-12 col-md-2 desctxt">{{parseInt(replay.duration/60,10)}} min</div>
        </div>
        <div class="clearfix">
          <div v-if="replay.epg2vdr" :id="'evImages' + replay.eventid" class="img-fluid float-right img-thumbnail carousel slide" data-ride="carousel" data-interval="5000">
            <div class="carousel-inner epg-image" role="listbox">
              <div v-for="n in imagecnt" class="carousel-item" :class="{'active':n==1}">
                <img class="d-block" :src="'/data/eventimg?id=' + replay.eventid + '&no=' + (n-1)" style="width: 100%;" alt="">
              </div>
            </div>
          </div>
          <div v-if="replay.epg2vdr">
            <div v-if="replay.epg2vdr.shorttext" class="card-text htxt">{{replay.shorttext}}</div>
            <div v-if="replay.epg2vdr.genre" class="card-text htxt">{{replay.epg2vdr.genre}}&nbsp;/&nbsp;{{replay.epg2vdr.category}}</div>
            <div v-if="replay.epg2vdr.episodepartname" class="card-text htxt">{{replay.epg2vdr.episodepartname}}</div>
            <div v-if="replay.epg2vdr.country" class="card-text htxt">{{replay.epg2vdr.country}}&nbsp;{{replay.epg2vdr.year}}</div>
            <div v-if="replay.epg2vdr.tipp" class="card-text htxt">{{replay.epg2vdr.tipp}}&nbsp;&nbsp;{{replay.epg2vdr.txtrating}}</div>
          </div>
          <div v-else="">
            <div v-if="replay.shorttext" class="card-text htxt">{{replay.shorttext}}</div>
          </div>
        <p class="desctxt" v-show="description" v-html="description"></p>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
export default {
    name: 'o2wReplay',
    props: {
        replay: Object
    },
    data(){
        return {
            current: null
        }
    },
    created() {
       this.$root.$on("replaycontrol", (data) => {
           this.current = data.current;
       });
    },
    computed: {
        description: function () {
            return this.replay.epg2vdr && this.replay.epg2vdr.longdescription ?
                this.replay.epg2vdr.longdescription.replace(/\n/g, '<br />') :
                this.replay.epg2vdr && this.replay.epg2vdr.shortdescription ?
                this.replay.epg2vdr.shortdescription.replace(/\n/g, '<br />') :
                this.replay.description ? this.replay.description.replace(/\n/g, '<br />') : '';
        },
        progress: function () {
            if (this.replay.title) {
                window.setTimeout(() => {
                    this.current += 60;
                }, 60000);
            }
            return Math.max(parseInt(this.current / this.replay.duration * 100,10), 0);
        },
        elapsed: function () {
            if (this.replay.title){
                window.setTimeout(() => {
                    this.now = parseInt(new Date().getTime() / 1000, 10);
                }, 60000);
            }
            return Math.max(parseInt((this.now - this.replay.starttime)/60,10),0);
        },
        imagecnt: function() {
            let cnt= this.replay.epg2vdr ? parseInt(this.replay.epg2vdr.imagecount, 10) : 0;
            return isNaN(cnt) ? 0 : cnt;
        }
    }
}
</script>
