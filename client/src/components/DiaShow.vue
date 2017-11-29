<template>
  <div class="diashow card mt-1">
    <div class="clearfix">
      <div :id="'diaImages' + diashow.filename" class="img-fluid float-right img-thumbnail carousel slide" data-ride="carousel" data-interval="5000">
        <div class="carousel-inner dia-image" role="listbox">
          <img class="d-block" :src="diashow.filename" style="width: 100%;" alt="">
        </div>
      </div>
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
