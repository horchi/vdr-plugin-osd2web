<template>
    <div>
        <o2w-fullevent v-if="isFullevent" :event="event" />
        <div v-else="">
            <div v-show="event.title" class="card mt-2" style="height:310px; overflow: hidden;">
               <div class="card-block p-1">
                 <div class="progress" v-show="progress">
                    <div class="progress-bar" role="progressbar" :style="{width: progress + '%'}" :aria-valuenow="{progress}" aria-valuemin="0" aria-valuemax="100">{{progress}}%</div>
                 </div>
                 <div class="row">
                   <h3 class="card-title col-12 col-md-9">{{$root.formatTime(event.starttime)}}&nbsp;{{event.title}}</h3>
                   <div class="ml-auto col-12 col-md-3">{{elapsed}}/{{parseInt(event.duration/60,10)}} min</div>
                 </div>
                 <div class="row">
                    <div v-if="event.epg2vdr" class="col-12 col-md-9">
                       <div v-if="event.epg2vdr.shorttext" class="card-text">{{event.shorttext}}</div>
                       <div v-if="event.epg2vdr.genre" class="card-text htxt">{{event.epg2vdr.genre}}</div>
                       <div v-if="event.epg2vdr.category" class="card-text htxt">{{event.epg2vdr.category}}</div>
                       <div v-if="event.epg2vdr.episodepartname" class="card-text">{{event.epg2vdr.episodepartname}}</div>
                       <div v-if="event.epg2vdr.country" class="card-text">{{event.epg2vdr.country}}&nbsp;{{event.epg2vdr.year}}</div>
                       <div v-if="event.epg2vdr.tipp" class="card-text">{{event.epg2vdr.tipp}}&nbsp;&nbsp;{{event.epg2vdr.txtrating}}</div>
                    </div>
                    <div v-else="" class="col-12 col-md-9">
                       <div v-if="event.shorttext" class="card-text">{{event.shorttext}}</div>
                    </div>
                    <div v-if="event.epg2vdr" :id="'evImages' + event.eventid" class="col-12 col-md-3 carousel slide" data-ride="carousel" data-interval="5000">
                      <div class="carousel-inner" role="listbox">
                        <div v-for="n in imagecnt" class="carousel-item" :class="{'active':n==1}">
                          <img class="d-block" :src="'/data/eventimg?id=' + event.eventid + '&no=' + (n-1)" alt="">
                        </div>
                      </div>
                      <a class="carousel-control-prev" :href="'#evImages' + event.eventid" role="button" data-slide="prev">
                        <span class="carousel-control-prev-icon" aria-hidden="true"></span>
                        <span class="sr-only">Previous</span>
                      </a>
                      <a class="carousel-control-next" :href="'#evImages' + event.eventid" role="button" data-slide="next">
                        <span class="carousel-control-next-icon" aria-hidden="true"></span>
                        <span class="sr-only">Next</span>
                      </a>
                    </div>
                 </div>
                 <p v-show="description" v-html="description"></p>
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
        description:  function () {
            return this.event.epg2vdr && this.event.epg2vdr.longdescription ? this.event.epg2vdr.longdescription.replace(/\n/g, '<br />') : this.event.epg2vdr && this.event.epg2vdr.shortdescription ? this.event.epg2vdr.shortdescription.replace(/\n/g, '<br />') : '';
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
        imagecnt: function(){
            let cnt= this.event.epg2vdr ? parseInt(this.event.epg2vdr.imagecount, 10) : 0;
            return isNaN(cnt) ? 0 : cnt;
        }
    }
}
</script>
