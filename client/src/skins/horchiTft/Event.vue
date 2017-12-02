<template>
  <o2w-fullevent v-if="isFullevent" :event="event" />
  <div v-else="" class="event card mt-1"
       v-bind:class="{ eventpresent : isPresent, eventfollowing : !isPresent }">
    <div v-show="event.title" style="height: 100%;">
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
          <h3 class="card-title col-12 col-md-9 titletxt">{{$root.formatTime(event.starttime)}}&nbsp;{{event.title}}</h3>
          <div v-if="elapsed > 0" class="ml-auto col-12 col-md-3 durationtxt">{{remaining}}/{{parseInt(event.duration/60,10)}} min</div>
          <div v-if="elapsed <= 0" class="ml-auto col-12 col-md-3 durationtxt">{{parseInt(event.duration/60,10)}} min</div>
        </div>
        <div class="clearfix">
          <div v-if="event.epg2vdr && imagecnt > 0" :id="'evImages' + event.eventid" class="img-fluid float-right epg-image carousel slide" data-ride="carousel" data-interval="5000">
            <div class="carousel-inner img-thumbnail" role="listbox">
              <div v-for="n in imagecnt" class="carousel-item" :class="{'active':n==0}">
                <img class="d-block" :src="'/data/eventimg?id=' + event.eventid + '&no=' + (n-1)" alt="">
              </div>
            </div>
          </div>
          <div v-if="event.epg2vdr">
            <div v-if="event.epg2vdr.episodepartname" class="card-text htxt">{{event.episodepartname}}</div>
            <div v-else="" class="card-text htxt">{{event.epg2vdr.shorttext}}</div>
            <div class="card-text htxt">{{event.epg2vdr.genre}}&nbsp;/&nbsp;{{event.epg2vdr.category}}&nbsp;/&nbsp;{{county_year}}</div>
<!--            <div v-if="event.epg2vdr.country" class="card-text htxt">({{event.epg2vdr.country}}&nbsp;{{event.epg2vdr.year}})</div> -->
            <div class="card-text htxt">{{rating}}</div>
          </div>
          <div v-else="">
            <div v-if="event.shorttext" class="card-text htxt">{{event.shorttext}}</div>
          </div>
        <p class="desctxt" v-show="description" v-html="description"></p>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
var common= require("common");
common.Vue.component('o2w-fullevent', require('../../components/Event'))

export default {
    name: 'o2wEvent',
    props: {
        event: Object,
        'isPresent': Object
    },
    data(){
        return {
            now: parseInt(new Date().getTime() / 1000, 10),
            isFullevent: this.$parent.$options._componentTag != 'o2w-actual'
        }
    },
    computed: {
        description: function () {
            return this.event.epg2vdr && this.event.epg2vdr.longdescription ?
                this.event.epg2vdr.longdescription.replace(/\n/g, '<br />') :
                  this.event.epg2vdr && this.event.epg2vdr.shortdescription ?
                  this.event.epg2vdr.shortdescription.replace(/\n/g, '<br />') :
                    this.event.description ? this.event.description.replace(/\n/g, '<br />') : '';
        },
        progress: function () {
            if (this.event.title){
                window.setTimeout(()=>{
                    this.now= parseInt(new Date().getTime() / 1000, 10);
                },60000);
            }
            return Math.max(parseInt((this.now - this.event.starttime) / this.event.duration * 100,10),0);
        },
        elapsed: function () {
            if (this.event.title){
                window.setTimeout(()=>{
                    this.now= parseInt(new Date().getTime() / 1000, 10);
                },60000);
            }
            return Math.max(parseInt((this.now - this.event.starttime)/60,10),0);
        },
        remaining: function () {
            if (this.event.title){
                window.setTimeout(()=>{
                    this.now= parseInt(new Date().getTime() / 1000, 10);
                },60000);
            }
            return Math.max(parseInt((this.event.duration - (this.now - this.event.starttime))/60,10),0);
        },
        county_year: function() {
            if (!this.event.epg2vdr)
                return "";

            this.event.epg2vdr.year = this.event.epg2vdr.year ? this.event.epg2vdr.year : "";
            this.event.epg2vdr.county = this.event.epg2vdr.county ? this.event.epg2vdr.county : "";

            return this.event.epg2vdr.country && this.event.epg2vdr.year ?
                this.event.epg2vdr.country + " " + this.event.epg2vdr.year :
                this.event.epg2vdr.country ? this.event.epg2vdr.country : this.event.epg2vdr.year;
        },
        rating: function() {
            if (!this.event.epg2vdr)
                return "";

            this.event.epg2vdr.txtrating = this.event.epg2vdr.txtrating ? this.event.epg2vdr.txtrating : "";
            this.event.epg2vdr.tipp = this.event.epg2vdr.tipp ? this.event.epg2vdr.tipp : "";

            return this.event.epg2vdr.tipp && this.event.epg2vdr.txtrating ?
                this.event.epg2vdr.tipp + " / " + this.event.epg2vdr.txtrating :
                this.event.epg2vdr.tipp ? this.event.epg2vdr.tipp :
                this.event.epg2vdr.txtrating;
        },

        imagecnt: function() {
            let cnt= this.event.epg2vdr ? parseInt(this.event.epg2vdr.imagecount, 10) : 0;
            return isNaN(cnt) ? 0 : cnt ;
        }
    },
    updated: function() {
        $('.carousel').carousel().each(function(){ $('.carousel-item',this).removeClass('active').first().addClass('active')});
    }
}
</script>
