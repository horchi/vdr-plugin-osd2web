<template>
   <div v-show="msg" :class="type">
       <a @click="msg=''" class="uk-alert-close"></a>
       <p>{{ msg }}</p>
   </div>
</template>

<script>

const eMessageType= [ "-success", "", "-warning", "-danger"];

export default {
    name: 'o2vStatusmessage',
    data() {
        return {
            msg: '',
            type: 0
        }
    },
    created() {
        this.$root.$on("message", (data) => {
            this.msg = data.message;
            this.type= "uk-alert" + (eMessageType[data.type] || '');
        })
    }
}
</script>
