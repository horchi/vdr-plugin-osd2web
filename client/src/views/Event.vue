<template>
   <div v-show="event.title" class="uk-panel uk-margin-left uk-margin-right">
       <h3 class="uk-panel-title">{{event.title}}</h3>
       <div class="uk-panel-teaser">{{event.shorttext}}</div>
       <div v-show="progress" class="uk-progress">
           <div class="uk-progress-bar" :style="'width:' + progress + '%'">{{progress}}%</div>
       </div>
       <div class="uk-flex uk-flex-space-between">
         <div class="uk-width-2-3 uk-panel">
            {{ $root.formatDateTime(event.starttime)}}&nbsp;-&nbsp;{{$root.formatTime(event.endtime)}}
         </div>
         <div class="uk-width-1-3 uk-panel uk-float-right">
            {{event.duration/60}} min</div>
       </div>
       <div class="uk-panel-box uk-margin-top">
          <div class="uk-grid uk-flex-top">
              <div v-if="event.eventid" class="uk-width-medium-1-3 uk-row-first">
                  <div class="uk-panel"><img :src="'/data/eventimg?id=' + event.eventid + '&no=0'" alt="" /></div>
              </div>
              <div class="uk-width-medium-2-3 uk-flex-middle">
                  <p v-show="description" v-html="description"></p>
              </div>
          </div>
       </div>
   </div>
</template>

<script>

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
       }
    }
}
</script>
