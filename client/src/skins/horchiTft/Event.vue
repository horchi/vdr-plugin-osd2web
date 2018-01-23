<template>
  <o2w-fullevent v-if="isFullevent" :event="event" />
  <div v-else="" class="event card mt-1"
       v-bind:class="{ 'eventpresent' : isPresent, 'eventfollowing' : !isPresent }">
    <div v-show="event.title" style="height: 100%;">
      <div class="card-body p-1" style="height: 100%;">
        <div class="progress" v-show="progress">
          <div class="progress-bar" role="progressbar"
               :style="{width: progress + '%'}"
               :aria-valuenow="{progress}"
               aria-valuemin="0"
               aria-valuemax="100">{{progress}}%
          </div>
        </div>
        <div class="eventtitlerow flexrow">
          <div class="titletime flexitem">{{$root.formatTime(event.starttime)}}</div>
          <div class="titletxt flexitemgrow auto-h-scroll">{{event.title}}</div>
          <div v-if="elapsed > 0" class="durationtxt flexitem">{{remaining}}/{{duration}}</div>
          <div v-if="elapsed <= 0" class="durationtxt flexitem">{{duration}}</div>
        </div>
        <div class="clearfix">
          <div v-if="event.epg2vdr && imagecnt > 0" :id="'evImages' + event.eventid" class="img-fluid float-right event-image-frame carousel slide" data-ride="carousel" data-interval="5000">
            <div class="carousel-inner img-thumbnail" role="listbox">
              <div v-for="n in imagecnt" class="carousel-item" :class="{'active':n==0}">
                <img class="d-block epg-image" :src="'/data/eventimg?id=' + event.eventid + '&no=' + (n-1)" alt="">
              </div>
            </div>
          </div>
          <div v-if="event.epg2vdr">
            <div v-if="event.epg2vdr.episodepartname" class="card-text subtitletxt">{{event.epg2vdr.episodepartname}}</div>
            <div v-else="" class="card-text subtitletxt">{{event.epg2vdr.shorttext}}</div>
          </div>
          <div v-else="">
            <div v-if="event.shorttext" class="card-text subtitletxt">{{event.shorttext}}</div>
          </div>
          <div class="card-text htxt">{{details1}}</div>
          <div class="card-text htxt">{{details2}}</div>
          <div v-show="description"><p class="desctxt auto-v-scroll" v-html="description"></p></div>
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
        'isPresent': Boolean
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
            return this.$root.formatDuration(Math.max(parseInt((this.event.duration - (this.now - this.event.starttime))/60,10),0));
        },
        duration: function () {
            return this.$root.formatDuration(parseInt(this.event.duration/60,10));
        },
        details1: function() {
            var text = "";
            if (!this.event.epg2vdr)
                return text;
            if (this.event.epg2vdr.category)
                text += this.event.epg2vdr.category;
            if (this.event.epg2vdr.genre)
                text += " / " + this.event.epg2vdr.genre
            if (this.event.epg2vdr.country)
                text += " / " + this.event.epg2vdr.country;
            if (this.event.epg2vdr.year)
                text += " " + this.event.epg2vdr.year;
            return text;
        },
        details2: function() {
            var text = "";
            if (!this.event.epg2vdr)
                return text;
            if (this.event.epg2vdr.tipp)
                text += this.event.epg2vdr.tipp;
            if (this.event.epg2vdr.txtrating)
                text += text.length > 0 ? " / " + this.event.epg2vdr.txtrating : this.event.epg2vdr.txtrating;
            if (this.event.epg2vdr.episodepart)
                text += (text.length > 0 ? " / Staffel " : "Staffel ") + this.event.epg2vdr.episodeseason + " Folge " + this.event.epg2vdr.episodepart + "/" + this.event.epg2vdr.episodeparts;
            return text;
        },
        series: function() {
            if (!this.event.epg2vdr || !this.event.epg2vdr.episodepart)
                return "";
            return "Staffel " + this.event.epg2vdr.episodeseason + "Folge " + this.event.epg2vdr.episodepart + "/" + this.event.epg2vdr.episodeparts;
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
