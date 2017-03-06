<template>
   <div v-show="msg" class="uk-alert-warning">
       <a @click="msg=''" class="uk-alert-close"></a>
       <p>{{ msg }}</p>
   </div>
</template>

<script>
export default {
    name: 'o2vStatusmessage',
    data() {
        return {
            msg: ''
        }
    },
    created() {
        this.$root.$on("message", (data) => {
            this.msg = data.message;
        })
    }
}
</script>
