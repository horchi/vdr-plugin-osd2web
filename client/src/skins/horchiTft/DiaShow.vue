<template>
  <div >
    <div class="diashow card mt-1">
      <div class="row">
        <h3 class="card-title col-12 col-md-9 titletxt">{{diashow.title}}</h3>
      </div>
      <div class="media my-auto d-flex flex-column ml-auto p-1 chlogo">
        <img class="d-block" :src="diashow.filename" />
      </div>
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
        return {
            current: 0
        }
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
