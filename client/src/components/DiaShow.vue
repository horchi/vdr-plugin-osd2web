<template>
  <div class="diashow">
    <img class="img-fluid" :src="diashow.filename">
  </div>
</div>
</template>

<script>
export default {
    name: 'o2wDiaShow',
    props: {
        diashow: Object
    },
    data() {
    },
    created() {
    },
    computed: {
    }
}
</script>
