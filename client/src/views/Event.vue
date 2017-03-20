<template>
   <div v-show="event.title" class="uk-panelt">
       <h3 class="uk-panel-title">{{event.title}}</h3>
       <div class="uk-panel-teaser">{{event.shorttext}}</div>
       <progress v-show="progress" class="uk-progress" :value="progress" max="100">{{progress}}%</progress>
       <div class="uk-flex">
         <div class="uk-width-2-3 uk-panel">
            {{ $root.formatDateTime(event.starttime)}}&nbsp;-&nbsp;{{$root.formatTime(event.endtime)}}
         </div>
         <div class="uk-width-1-3 uk-panel uk-float-right">
            {{parseInt(event.duration/60,10)}} min</div>
       </div>
       <div v-if="event.epg2vdr" class="uk-panel uk-margin-top">
          <img v-for="n in images" class="uk-align-left uk-margin-remove-adjacent" :src="'/data/eventimg?id=' + event.eventid + '&no=' + (n-1)" alt="" />
          <p v-html="event.epg2vdr.longdescription"></p>
       </div>
       <hr class="uk-divider-icon" />
       <div v___-else class="uk-panel uk-margin-top">
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
              "shortdescription",    //    mtext   3000
              "actor",               //    mtext   5000
              "longdescription",     //    mtext  25000
              "cntlongdescription",  //    MText  25000

*/
export default {
    name: 'o2vEvent',
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
       images: function(){
         let cnt= parseInt(this.event.epg2vdr.imagecount, 10);
         return isNaN(cnt) ? 0 : cnt;
       }
    }
}
</script>
