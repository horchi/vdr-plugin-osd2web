<template>
   <div v-show="event.title" class="uk-panel uk-margin-left uk-margin-right">
       <h3 class="uk-panel-title">{{event.title}}</h3>
       <div class="uk-panel-teaser">{{event.shorttext}}</div>
       <div class="uk-progress">
           <div class="uk-progress-bar" :style="getProgress"></div>
       </div>
       <div class="uk-flex uk-flex-space-between">
         <div class="uk-width-2-3 uk-panel">
            {{ $root.formatDateTime(event.starttime)}}&nbsp;-&nbsp;{{$root.formatTime(event.endtime)}}
         </div>
         <div class="uk-width-1-3 uk-panel uk-float-right">
            {{event.duration/60}} min</div>
       </div>
       <div v-show="event.description" class="uk-panel-box uk-grid-margin" v-html="getDescription"></div>
   </div>
</template>

<script>

export default {
    name: 'o2vEvent',
    props: {
      event: Object
    },
    computed: {
      getDescription: function () {
         return this.event.description ? this.event.description.replace(/\n/g, '<br />') : '';
      },
      getProgress: function(){
         return 'width:' + ((parseInt(new Date().getTime() / 1000, 10) - this.event.starttime) / this.event.duration * 100) + '%';
      }
    }
}
</script>
