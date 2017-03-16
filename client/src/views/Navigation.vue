<template>
   <div id="topnav" style="width:100%">
      <nav class="uk-navbar-container" uk-navbar style="position:fixed; z-index:9999;width:100%;">
          <!--a @click="show_level0= !show_level0" class="uk-navbar-toggle uk-visible-small"></a>
          <ul class="uk-navbar-nav" :class="{'uk-hidden-small':!show_level0}">-->
        <div class="uk-navbar-left">
          <ul class="uk-navbar-nav">
              <li v-for="item in items" @click="handleSelect(item)" v-if="!item.hidden" :class="{'uk-active':item.on}">
                  <a v-html="renderLabel(item)"></a>
              </li>
          </ul>
        </div>
        <div class="uk-navbar-right">
          <ul class="uk-navbar-nav">
             <li v-for="item in itemsRight" @click="handleSelect(item)" :class="{'uk-active':item.on}">
                  <a v-html="renderLabel(item)"></a>
             </li>
          </ul>
        </div>
      </nav>
   </div>
</template>

<script>
import UIkit from 'uikit';
import Icons from 'uikit/dist/js/uikit-icons';

UIkit.use(Icons);

export default {
    name: 'o2vNavigation',
    props: {
      show_level0: true,
      'items': Array,
      'itemsRight': Array,
    },
    methods: {
        checkHeight(){
           let topnav= document.getElementById('topnav');
           if (topnav)
            topnav.style.height= topnav.firstChild.offsetHeight + 'px';
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
            return (item.icon ? '<span uk-icon="icon: ' + item.icon + '"></span>' : '') + label;
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
