<template>
    <div v-if="channel.channelid">
        <div class="card-header">
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
        })
    }
}
</script>

