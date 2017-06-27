<template>
  <o2w-fullevent v-if="isFullevent" :event="event" />
  <div v-else="" class="event card mt-1">
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
          <div v-if="elapsed > 0" class="ml-auto col-12 col-md-3 desctxt">{{elapsed}}/{{parseInt(event.duration/60,10)}} min</div>
          <div v-if="elapsed <= 0" class="ml-auto col-12 col-md-3 desctxt">{{parseInt(event.duration/60,10)}} min</div>
        </div>
        <div class="clearfix">
          <div v-if="event.epg2vdr && imagecnt > 0" :id="'evImages' + event.eventid" class="img-fluid float-right epg-image carousel slide" data-ride="carousel" data-interval="5000">
            <div class="carousel-inner img-thumbnail" role="listbox">
              <div v-for="n in imagecnt" class="carousel-item" :class="{'active':n==1}">
                <img class="d-block" :src="'/data/eventimg?id=' + event.eventid + '&no=' + (n-1)" alt="">
              </div>
            </div>
          </div>
          <div v-if="event.epg2vdr">
            <div v-if="event.epg2vdr.shorttext" class="card-text htxt">{{event.shorttext}}</div>
            <div v-if="event.epg2vdr.genre" class="card-text htxt">{{event.epg2vdr.genre}}&nbsp;/&nbsp;{{event.epg2vdr.category}}</div>
            <div v-if="event.epg2vdr.episodepartname" class="card-text htxt">{{event.epg2vdr.episodepartname}}</div>
            <div v-if="event.epg2vdr.country" class="card-text htxt">{{event.epg2vdr.country}}&nbsp;{{event.epg2vdr.year}}</div>
            <div v-if="event.epg2vdr.tipp" class="card-text htxt">{{event.epg2vdr.tipp}}&nbsp;&nbsp;{{event.epg2vdr.txtrating}}</div>
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
        event: Object
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
        imagecnt: function() {
            let cnt= this.event.epg2vdr ? parseInt(this.event.epg2vdr.imagecount, 10) : 0;
            return isNaN(cnt) ? 0 : cnt;
        }
    },
      updated:function() {
        $('.carousel').carousel().each(function(){ $('.carousel-item',this).removeClass('active').first().addClass('active')});
    }
}
</script>
