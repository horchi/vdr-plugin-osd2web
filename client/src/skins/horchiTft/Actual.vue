<template>
  <div id="Actual">
    <div v-if="replay_active">
      <o2w-replay :replay="replay"></o2w-replay>
    </div>
    <div v-else="">
      <div class="channel flexrow mt-1">
        <div class="chnr flexitem">{{channel.channelnumber}}</div>
        <div class="chtxt flexitemgrow">
          <p>{{channel.channelname}}</p>
        </div>
        <div v-if="logoname" class="chlogo flexitem">
          <img class="chlogo" :src="'/data/channellogo?name=' + logoname + '&id=' + channel.channelid" />
        </div>
      </div>
      <div v-if="following">
        <o2w-event :event="present" :isPresent="true"></o2w-event>
        <div class="eventsep"/>
        <o2w-event :event="following" :isPresent="false"></o2w-event>
      </div>
      <div v-else="">
        <o2w-event :event="present" :channel="channel" :isPresent="true" :isRadio="true"></o2w-event>
      </div>
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
            return this.$root.hasChannelLogos && this.channel.channelname ? encodeURIComponent(this.channel.channelname) : '';
        }
    }
}
</script>
