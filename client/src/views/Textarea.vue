<template>
  <div v-show="text" class="uk-panel-box uk-grid-margin" v-html="text"></div>
</template>
<script>
export default {
    name: 'o2vTextarea',
    data: function() {
        return {
            'text': ''
        }
    },
    created() {
        this.$root.$on("menutext", (data) => {
            this.text = data.text.replace(/\n/g, '<br />');
        });
        this.$root.$on("textscroll", (data) => {
             if (data.page) {
                 if (data.up)
                     document.body.scrollLeft = Math.max(0, document.body.scrollLeft - document.body.clientWidth);
                 else
                     document.body.scrollLeft += document.body.clientWidth;
             } else {
                 if (data.up)
                     document.body.scrollTop = Math.max(0, document.body.scrollTop - document.body.clientHeight);
                 else
                     document.body.scrollTop = document.body.scrollTop + document.body.clientHeight;
             }
        });
    }
}
</script>
