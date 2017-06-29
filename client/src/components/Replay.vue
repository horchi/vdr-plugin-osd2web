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
  </div>
</template>

<script>
export default {
    name: 'o2wReplay',
    props: {
        replay: Object
    },
    data() {
        return {
            current: 0
        }
    },
    created() {
        this.$root.$on("replaycontrol", (data) => {
            this.current = data.current;
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
        }
    },
    updated: function() {
        $('.carousel').carousel().each(function(){ $('.carousel-item',this).removeClass('active').first().addClass('active')});
    }
}
</script>
