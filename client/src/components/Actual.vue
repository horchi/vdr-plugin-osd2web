<template>
  <div id="Actual">
    <div v-if="channel.channelid" class="channel card mt-1">
      <div class="row" style="height: 100%;">
        <h3 v-if="!replay_active" class="d-block my-auto ml-3 card-title htxt chtxt">&nbsp;{{channel.channelnumber}}</h3>
        <h3 class="d-block my-auto card-title chtxt">&nbsp;{{channel.channelname}}</h3>
        <div v-if="$root.hasChannelLogos" class="media my-auto d-flex flex-column ml-auto p-1 chlogo">
          <img align="right" class="d-block" style="max-height: 95%" :src="'/data/channellogo?name=' + logoname + '&id=' + channel.channelid" />
        </div>
      </div>
    </div>
    <div v-if="replay_active">
      <o2w-replaycontrol />
      <o2w-replay :replay="replay"></o2w-replay>
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
            replay_active: null,
            channel: {},
            present: {},
            following: {},
            replay: {}
        }
    },
    created() {
        this.$root.$on("actual", (data) => {
            this.channel = data.channel;
            this.present = data.present;
            this.following = data.following;
        });
        this.$root.$on("replay", (data) => {
            this.replay_active = data.active;
            if (data.info) {
                this.channel.channelid = data.info.channelid;
                this.channel.channelname = data.info.channelname;
                this.replay = data;
            } else
                this.replay = null;
        });
    },
    computed: {
        logoname: function () {
            return encodeURIComponent(this.channel.channelname);
        }
    }
}
</script>
