<template>
  <div v-if="recordings" class="list-group" id="actual-recordings" style="overflow: hidden;">
    <div v-for="(recording,n) in recordings"
         class="recording mt-1 list-group-item card flex-column p-0 active">
      <div class="w-100 justify-content-between rectxt">
        <div class="rltmtxt">{{$root.formatDateTimeShort(recording.start)}}</div>
        <div>{{recording.name}}</div>
      </div>
    </div>
  </div>
</template>
<script>

export default {
    name: 'o2wRecording',
    data: function () {
        return {
            recordings: null
        };
    },
    created() {
        this.$root.$on("recordings", (data) => {
            this.recordings = data && data.length ? data : null;
        });
    }
}
/*
{
   "event" : "recordings",
   "object" : [
      {
         "name" : "Das Vermächtnis der Wanderhure",
         "folder" : "",
         "title" : "19.09.15 23:48  Das Vermächtnis der Wanderhure",
         "hasmarks" : 1,
         "basename" : "Das Vermächtnis der Wanderhure",
         "start" : 1442699401,
         "lengthinseconds" : 8517,
         "isedited" : 0,
         "event" : {
            "vps" : 0,
            "starttime" : 1442699401,
            "eventid" : 668670,
            "runningstatus" : 0,
            "endtime" : 1442707201,
            "timermatch" : "none",
            "duration" : 7800,
            "components" : [
               {
                  "description" : "",
                  "language" : "deu",
                  "type" : 5,
                  "stream" : 2
               }
            ],
            "hastimer" : 0,
            "seen" : 1498644005,
            "parentalrating" : 12,
            "title" : "Das Vermächtnis der Wanderhure",
            "isrunning" : 0,
            "shorttext" : "Drama (D/CZ 2012)"
         },
         "info" : {
            "channelid" : "S19.2E-1-1017-61303",
            "channelname" : "SIXX HD",
            "framespersecond" : 25
         },
         "filename" : "/tank/video/Das_Vermächtnis_der_Wanderhure/2015-09-19.23.48.12-0.rec",
         "isnew" : 1,
         "filesizemb" : 9272
      },
      {
         "isedited" : 0,
         "event" : {
            "duration" : 5700,
            "components" : [
               {
                  "description" : "Deutsch",
                  "language" : "deu",
                  "stream" : 4,
                  "type" : 44
               },
               {
                  "language" : "deu",
                  "description" : "Englisch",
                  "type" : 44,
                  "stream" : 4
...
...

   ]
}
*/
</script>
