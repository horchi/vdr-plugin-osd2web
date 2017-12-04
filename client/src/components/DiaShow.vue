<template>
  <div class="diashow">
    <img id="diaShow_img1" />
    <img id="diaShow_img2" style="display:none" />
  </div>
</div>
</template>

<script>

var dia={
    $imgNext: null,
    $imgCur: null,
    useFirstImage: true,
    setImage: function(diashow){
      if (this.useFirstImage){
        this.useFirstImage= false;
        this.$imgCur= $('#diaShow_img2');
        this.$imgNext= $('#diaShow_img1');
      } else{
        this.useFirstImage= true;
        this.$imgCur= $('#diaShow_img1');
        this.$imgNext= $('#diaShow_img2');
      }
      this.$imgNext.prop("src",diashow.filename).one("load",this, this.effects[diashow.effect] || this.effects['fade']);
    },
    effects: {
      "hard": function(ev){
          ev.data.$imgNext.show();
          ev.data.$imgCur.hide();
      },
      // animations sind in common.scss definiert
      "fade": function(ev){
          ev.data.$imgNext[0].style.cssText= "animation: 3s fadeIn";
          ev.data.$imgCur[0].style.cssText= "animation: 3s fadeOut";
      },
      "slideLeft": function(ev){
          ev.data.$imgNext[0].style.cssText= "animation: 3s slideLeftIn";
          ev.data.$imgCur[0].style.cssText= "animation: 3s slideLeftOut";
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
      dia.setImage(this.diashow);
    },
    watch: {
      diashow: function(){
        dia.setImage(this.diashow);
      }
    }
}
</script>
