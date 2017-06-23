<template>
  <div id="Actual">
    <o2w-replaycontrol />
    <div v-if="channel.channelid" class="card-header">
      <div class="text-center row">
        <div class="col media d-flex flex-column">
          <img v-if="$root.hasChannelLogos" class="d-block img-fluid" :src="'/data/channellogo?name=' + channel.channelname + '&id=' + channel.channelid" />
          <h3 class="d-block mt-4 card-title">{{channel.channelname}}</h3>
        </div>
        <div class="col">
          <div class="o2wChannelCircle alert-success ml-3">{{channel.channelnumber}}</div>
        </div>
      </div>
    </div>
    <div v-if="replay.name">
      <o2w-replay :event="replay"></o2w-replay>
    </div>
    <div v-else="">
      <o2w-event :event="present"></o2w-event>
      <o2w-event :event="following"></o2w-event>
    </div>
    </div>
</template>

<script>
export default {
    name: 'o2wActual',
    data() {
        return {
            channel:{},
            present: {},
            following: {},
            replay: {}
        }
    },
    created() {
        this.$root.$on("actual", (data) => {
            this.replay = {};
            this.channel = data.channel;
            this.present= data.present;
            this.following= data.following;
        });
        this.$root.$on("replay", (data) => {
            this.channel = {};
            this.present = {};
            this.following = {};
            let ev = data.event || {};
            ev.name = data.name;
            if (!ev.name) {
                ev.name = "<null>";
            }
            if (!data.event) {
                ev.title = data.name;
                ev.duration = data.lengthinseconds;
            }
            ev.starttime = parseInt(new Date().getTime() / 1000,10);
            ev.endtime = ev.starttime + ev.duration;
            this.replay = ev;
        });
    }
}
</script>
