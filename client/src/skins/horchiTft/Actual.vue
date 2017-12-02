<template>
  <div id="Actual">
    <div class="channel card mt-1">
      <div class="row" style="height: 100%;">
        <h3 v-if="!replay_active" class="col-12 col-sm-1 d-block my-auto card-title chtxt">&nbsp;{{channel.channelnumber}}</h3>
        <h3 class="col-12 col-sm-6 d-block my-auto card-title chtxt">&nbsp;{{channel.channelname}}</h3>
        <div v-if="$root.hasChannelLogos" class="col-12 col-sm-5 my-auto d-flex flex-column ml-auto" style="height: 100%;">
          <img class="chlogo d-block" :src="'/data/channellogo?name=' + logoname + '&id=' + channel.channelid" />
        </div>
      </div>
    </div>
    <div v-if="replay_active">
      <o2w-replay :replay="replay"></o2w-replay>
    </div>
    <div v-else="">
      <o2w-event :event="present" :isPresent="true"></o2w-event>
      <div class="eventsep"/>
      <o2w-event :event="following" :isPresent="fasle"></o2w-event>
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
            if (data.info){
                this.replay_active = data.active;
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
