<template>
  <div >
    <div class="diashow card mt-1">
      <div v-show="diashow.title" style="height: 100%;">
        <div class="card-block p-1" style="height: 100%;">
        </div>
        <div class="row">
          <h3 class="card-title col-12 col-md-9 titletxt">{{diashow.title}}</h3>
        </div>
        <div class="clearfix">
          <div :id="'diaImages' + diashow.filename" class="img-fluid float-right img-thumbnail carousel slide" data-ride="carousel" data-interval="5000">
            <div class="carousel-inner dia-image" role="listbox">
              <div v-for="(img, n) in diashow.images" class="carousel-item" :class="{'active':n==0}">
                <img class="d-block" :src="diashow.filename" style="width: 100%;" alt="">
              </div>
            </div>
          </div>
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
