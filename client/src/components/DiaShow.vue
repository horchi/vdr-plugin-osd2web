<template>
  <div class="diashow">
    <img id="diaShow_img1" class="diaimg" />
    <img id="diaShow_img2" class="diaimg" style="display:none" />
  </div>
</div>
</template>

<script>

var dia={
    imgCur: null,
    imgNext: null,
    init: function(){
        this.imgCur= $('#diaShow_img1').on("load", this.__imageloaded)[0];
        this.imgNext= $('#diaShow_img2').on("load", this.__imageloaded)[0];
        this.imgCur.dia= this.imgNext.dia= this;
    },
    setImage: function(diashow){
      this.imgNext.diaData= diashow;
      this.imgNext.src= diashow.filename;
    },
    __imageloaded: function(){
        if ( this.dia.imgNext == this){
          var f= this.dia.effects[this.diaData.effect] || this.dia.effects['fade'];
          f && f.apply(this.dia);
          this.dia.imgNext= this.dia.imgCur;
          this.dia.imgCur= this;
        }
    },
    effects: {
      "hard": function(ev){
          $(this.imgNext).show();
          $(this.imgCur).hide();
      },
      // animations sind in common.scss definiert
      "fade": function(ev){
          this.imgNext.style.cssText= "opacity:1; animation: 3s fadeIn";
          this.imgCur.style.cssText= "opacity:0; animation: 3s fadeOut";
      },
      "slideLeft": function(ev){
          this.imgNext.style.cssText= "left:0; animation: 3s slideLeftIn";
          this.imgCur.style.cssText= "left:100%; animation: 3s slideLeftOut";
      }
    }
}

export default {
    name: 'o2wDiaShow',
    props: {
      diashow: Object
    },
    data() {
      return { }
    },
    created(){
      var allEffects= [];
      for ( var eff in dia.effects)
        allEffects.push(eff);
      this.$root.$emit("send", {
          "event": "diaEffects",
          object: allEffects
      });
    },
    mounted(){
      dia.init();
      dia.setImage(this.diashow);
    },
    watch: {
      diashow: function(){
        dia.setImage(this.diashow);
      }
    }
}
</script>
