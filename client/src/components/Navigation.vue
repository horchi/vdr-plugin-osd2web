<template>

  <nav class="navbar navbar-toggleable-xl navbar-light bg-faded fixed-top">
    <button class="navbar-toggler navbar-toggler-right" type="button" data-toggle="collapse" data-target="#navbar"
            aria-controls="navbar" aria-expanded="false" aria-label="Toggle navigation">
      <span class="navbar-toggler-icon"></span>
    </button>
    <!--<a class="navbar-brand" href="#">Navbar w</a>-->
    <div class="collapse navbar-collapse" id="navbar">
        <ul class="navbar-nav mr-auto">
            <li v-for="item in items" @click="handleSelect(item)" v-if="!isHidden(item)" class="nav-item" :class="{'active':item.on}">
                <a class="nav-link" href="#"><icon v-if="item.icon" :name="item.icon" /><span v-html="renderLabel(item)"></span></a>
            </li>
        </ul>
        <ul class="navbar-nav ml-auto" v-if="$root.isActive">
            <li v-for="item in itemsRight" @click="handleSelect(item)" v-if="!isHidden(item)" class="nav-item" :class="{'active':item.on}">
                <a class="nav-link" href="#"><icon v-if="item.icon" :name="item.icon" /><span v-html="renderLabel(item)"></span></a>
            </li>
        </ul>
    </div>
  </nav>


</template>

<script>
export default {
    name: 'o2wNavigation',
    props: {
      'items': Array,
      'itemsRight': Array
    },
    methods: {
        checkHeight(){
           let topnav= document.getElementById('topnav');
           if (topnav)
            topnav.style.height= topnav.firstChild.offsetHeight + 'px';
        },
        isHidden(item){
            return typeof item.isHidden == "function" ? item.isHidden.call(item, this) : item.isHidden;
        },
        handleSelect(item) {
            if (item) {
                if (item.func)
                    item.func.call(item, this);
                else if (item.key)
                    this.$root.sendKey(item.key);
            }
        },
        renderLabel(item) {
            let key = null;
            let label = item.label.replace(/_(.)_/, function(a, k) {
                key = k.toLowerCase();
                return '<u>' + k + '</u>';
            });
            if (key)
                this.keys[key] = item;
            return label;
        }
    },
    mounted() {
        window.setTimeout(this.checkHeight,300);
    },
    updated() {
        this.checkHeight();
    },
    created() {
        this.keys = {};
        window.addEventListener('keyup', (ev) => {
            if (ev.altKey && this.keys[ev.key]) {
                this.handleSelect(this.keys[ev.key]);
                ev.stopPropagation()
            }
        });
        window.addEventListener('resize', this.checkHeight);
    }
}
</script>
