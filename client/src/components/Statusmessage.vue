<template>
   <div v-show="msg" class="alert alert-dismissible fade show" :class="type">
       <button @click="msg=''" type="button" class="close" data-dismiss="alert" aria-label="Close">
            <span aria-hidden="true">&times;</span>
        </button>
       <p>{{ msg }}</p>
   </div>
</template>

<script>

const eMessageType= [ "-success", "-info", "-warning", "-danger"];

export default {
    name: 'o2wStatusmessage',
    data() {
        return {
            msg: '',
            type: 0
        }
    },
    created() {
        this.$root.$on("message", (data) => {
            this.msg = data.message;
            this.type= "alert" + (eMessageType[data.type] || '');
        })
        this.$root.$on("channelgroup", (data) => {
            this.msg = data.name;
            this.type= "alert-info";
        })
    }
}
</script>
