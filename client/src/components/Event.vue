<template>
   <div v-show="event.title" class="card">
      <div class="card-block">
        <h3 class="card-title">{{event.title}}</h3>
        <div class="card-text">{{event.shorttext}}</div>
        <div class="progress" v-show="progress">
          <div class="progress-bar" role="progressbar" :style="{width: progress + '%'}" :aria-valuenow="{progress}" aria-valuemin="0" aria-valuemax="100">{{progress}}%</div>
        </div>
        <div class="row">
          <div class="col">
              {{ $root.formatDateTime(event.starttime)}}&nbsp;-&nbsp;{{$root.formatTime(event.endtime)}}
          </div>
          <div class="col text-right">
              {{parseInt(event.duration/60,10)}} min</div>
        </div>
        <div v-if="event.epg2vdr" :id="'evImages' + event.eventid" class="carousel slide" data-ride="carousel" data-interval="5000">
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
           <p v-html="event.epg2vdr.longdescription"></p>
        </div>
<!--
        <div v-if="event.epg2vdr" class="row">
            <img v-for="n in images" class="mx-auto d-block mb-1" :src="'/data/eventimg?id=' + event.eventid + '&no=' + (n-1)" alt="" />
            <p v-html="event.epg2vdr.longdescription"></p>
        </div>
        -->
      </div>
      <hr />
      <div class="card-block">
         <p v-show="description" v-html="description"></p>
      </div>
   </div>
</template>

<script>

/* event.epg2vdr:

       "imagecount",          //    int
       "numrating",           //    int
       "year",                //    ascii     10
       "category",            //    ascii     50
       "country",             //    ascii     50
       "audio",               //    ascii     50

       "txtrating",           //    ascii    100
       "genre",               //    ascii    100
       "flags",               //    ascii    100
       "commentator",         //    ascii    200
       "tipp",                //    ascii    250
       "rating",              //    ascii    250
       "moderator",           //    ascii    250
       "music",               //    ascii    250
       "screenplay",          //    ascii    500
       "shortreview",         //    ascii    500

       "guest",               //    text    1000
       "producer",            //    text    1000
       "camera",              //    text    1000
       "director",            //    text    1000
       "topic",               //    ascii   1000

       "other",               //    text    2000
       "actor",               //    mtext   5000
       "longdescription",     //    mtext  25000

*/
export default {
    name: 'o2wEvent',
    props: {
      event: Object
    },
    data(){
      return {
         now: parseInt(new Date().getTime() / 1000, 10)
      }
    },
    computed: {
       description:  function () {
          return this.event.description ? this.event.description.replace(/\n/g, '<br />') : '';
       },
       progress: function () {
          if (this.event.title){
              window.setTimeout(()=>{
                  this.now= parseInt(new Date().getTime() / 1000, 10);
             },60000);
         }
         return Math.max(parseInt((this.now - this.event.starttime) / this.event.duration * 100,10),0);
       },
       imagecnt: function(){
         let cnt= this.event.epg2vdr ? parseInt(this.event.epg2vdr.imagecount, 10) : 0;
         return isNaN(cnt) ? 0 : cnt;
       }
    }
}
</script>
