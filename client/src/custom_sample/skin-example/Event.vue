<template>
   <div v-show="event.title" class="card">
      <div class="card-body">
        <h3 class="card-title">{{event.title}}</h3>
        <div class="row">
          <div class="col">
              {{ $root.formatDateTime(event.starttime)}}&nbsp;-&nbsp;{{$root.formatTime(event.endtime)}}
          </div>
          <div class="col text-right">
              {{parseInt(event.duration/60,10)}} min</div>
        </div>
        <div class="card-text">{{event.shorttext}}</div>
        <div class="progress" v-show="progress">
          <div class="progress-bar" role="progressbar" :style="{width: progress + '%'}" :aria-valuenow="{progress}" aria-valuemin="0" aria-valuemax="100">{{progress}}%</div>
        </div>
        <div v-if="event.epg2vdr" class="row">
            <img v-for="n in images" class="mx-auto d-block mb-1" :src="'/data/eventimg?id=' + event.eventid + '&no=' + (n-1)" alt="" />
            <p v-html="event.epg2vdr.longdescription"></p>
        </div>
      </div>
      <hr />
      <div v__-else class="card-body">
         <p v-show="description" v-html="description"></p>
      </div>
   </div>
</template>

<script>
import oEvent from '../../components/Event.vue'
export default {
    name: 'o2wEvent',
    extends: oEvent
}

</script>
