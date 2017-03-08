<template>
<div v-if="channel.channelid" class="uk-panel">
   <div class="uk-panel uk-panel-space">
      <div class="o2vChannelCircle uk-alert-success uk-float-right">{{channel.channelnumber}}</div>
      <h3 class="uk-panel-title">{{channel.channelname}}</h3>
   </div>
   <o2v-event :event="present"></o2v-event>
   <o2v-event :event="following" class="uk-panel-divider"></o2v-event>
</div>
</template>

<script>
import o2vEvent from './Event.vue';
export default {
    name: 'o2vActual',
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
        })
    },
    components: {
        'o2v-event': o2vEvent
    }
}
</script>
<style>

.o2vChannelCircle  {
    width:120px;
    height:120px;
    line-height:120px;
    border-radius:60px;
    font-size:50px;
    text-align:center;
    }
</style>
