<template>
    <div v-if="channel.channelid" class="uk-card uk-card-default uk-width-1-1">
        <div class="uk-card-header uk-text-center">
            <div class="uk-float-right o2vChannelCircle uk-alert-success">{{channel.channelnumber}}</div>                
            <div v-if="$root.hasChannelLogos" class="uk-float-left">
                <img :src="'/data/channellogo?name=' + channel.channelname" />
            </div>
            <div class="uk-float-left uk-margin-left uk-margin-right">
                <h3 class="uk-card-title">{{channel.channelname}}</h3>
            </div>
        </div>
        <div class="uk-card-body">
            <o2v-event :event="present"></o2v-event>
        </div>
        <div class="uk-card-footer">
            <o2v-event :event="following"></o2v-event>
        </div>
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
