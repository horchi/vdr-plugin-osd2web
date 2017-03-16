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
            {{event.duration/60}} min</div>
       </div>
       <div class="uk-panel uk-margin-top">
          <img v-if="event.eventid" class="uk-align-left uk-margin-remove-adjacent" :src="'/data/eventimg?id=' + event.eventid + '&no=0'" alt="" />
          <p v-show="description" v-html="description"></p>
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
