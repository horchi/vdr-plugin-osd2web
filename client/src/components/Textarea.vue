<template>
  <div v-show="text" class="textmenu card">
      <div class="card-body" v-html="text"></div>
  </div>
</template>
<script>
export default {
    name: 'o2wTextarea',
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
