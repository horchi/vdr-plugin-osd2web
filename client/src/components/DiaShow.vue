<template>
  <div class="diashow">
    <img class="img-fluid float-right carousel slide" :src="diashow.filename" style="width: 100%;" alt="">
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
    },
    updated: function() {
        $('.carousel').carousel().each(function(){ $('.carousel-item',this).removeClass('active').first().addClass('active')});
    }
}
</script>
