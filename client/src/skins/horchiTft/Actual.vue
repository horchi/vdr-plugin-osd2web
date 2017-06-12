<template>
  <div id="Actual">
    <o2w-replaycontrol />
    <div v-if="channel.channelid" class="channel card mt-1">
      <div class="row" style="height: 100%;">
        <h3 class="d-block my-auto ml-3 card-title htxt chtxt">&nbsp;{{channel.channelnumber}}</h3>
        <h3 class="d-block my-auto card-title chtxt">&nbsp;{{channel.channelname}}</h3>
        <div v-if="$root.hasChannelLogos" class="media my-auto d-flex flex-column ml-auto p-1 chlogo">
          <img align="right" class="d-block" style="max-height: 95%" :src="'/data/channellogo?name=' + channel.channelname + '&id=' + channel.channelid" />
        </div>
      </div>
    </div>
    <o2w-event :event="present"></o2w-event>
    <o2w-event :event="following"></o2w-event>
  </div>
</template>

<script>
export default {
    name: 'o2wActual',
    data() {
        return {
            channel:{},
            present: {},
            following: {}
        }
    },
    created() {
        this.$root.$on("actual", (data) => {
            this.channel = data.channel;
            this.present= data.present;
            this.following= data.following;
        });
        this.$root.$on("replay", (data) => {
            this.channel = {};
            this.following= {};
            let ev= data.event || {};
            if (!data.event){
                ev.title= data.name;
                ev.duration= data.lengthinseconds;
            }
            ev.starttime= parseInt(new Date().getTime() / 1000,10);
            ev.endtime= ev.starttime + ev.duration;
            this.present= ev;
        });
    }
}
</script>
