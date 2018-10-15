<template>
   <div v-show="event.title" class="card textmenu">
      <div class="card-body">
        <div class="progress" v-show="progress">
          <div class="progress-bar" role="progressbar" :style="{width: progress + '%'}" :aria-valuenow="{progress}" aria-valuemin="0" aria-valuemax="100">{{progress}}%</div>
        </div>
        <div class="row">
          <div class="col" style="padding: 0px;"> {{ $root.formatTime(event.starttime)}} </div>
          <div class="col text-right" style="padding: 0px;"> {{parseInt(event.duration/60,10)}} min</div>
        </div>
        <div v-if="!isRadio && event.epg2vdr && imagecnt > 0" :id="'evImages' + event.eventid" class="img-fluid float-right event-image-frame carousel slide" data-ride="carousel" data-interval="5000">
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
        <p v-show="description" v-html="description"></p>
      </div>
      <hr />
   </div>
</template>

<script>

export default {
    name: 'o2wRightEvent',
    props: {
      event: Object
    },
    data(){
      return {
         now: parseInt(new Date().getTime() / 1000, 10)
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
       imagecnt: function(){
         let cnt= this.event.epg2vdr ? parseInt(this.event.epg2vdr.imagecount, 10) : 0;
         return isNaN(cnt) ? 0 : cnt;
       }
    },
    updated: function() {
        $('.carousel').carousel().each(function(){ $('.carousel-item',this).removeClass('active').first().addClass('active')});
    }
}
</script>
