<template>
    <div v-show="title" id="osdCon">
        <div class="uk-panel-box-primary uk-panel-hover uk-padding-bottom-remove uk-padding-top-remove" @click="$root.sendKey('Back')">
            <h3 class="uk-panel-title uk-margin-bottom-remove"><i v-if="!$root.isOnlyView" uk-icon="icon: chevron-left"></i> {{ title }}</h3>
            <!--<ul v-if="pageCount" class="uk-pagination">
                <li :class="{'uk-disabled': pageCurrent <= 0}" @click="$root.sendKey('Left')"><span><i class="uk-icon-angle-double-left"></i></span></li>
                <li v-for="p in pageCount" :class="{'uk-active':p == pageCurrent}"><a>{{p}}</a></li>
                <li :class="{'uk-disabled': pageCurrent >= pageCount}" @click="$root.sendKey('Right')"><span><i class="uk-icon-angle-double-right"></i></span></li>
            </ul>
            <ul class="uk-pagination">
                
                <li class="uk-active"><span>1</span></li>
                <li><a href="#">2</a></li>
                <li><a href="#">3</a></li>
                <li><a href="#">4</a></li>
                <li><span>...</span></li>
                <li><a href="#">20</a></li>
                <li><a href="#"><i class="uk-icon-angle-double-right"></i></a></li>
            </ul>-->
        </div>
        <o2v-textmenu></o2v-textmenu>
        <o2v-event :event="event"></o2v-event>
        <o2v-textarea></o2v-textarea>
        <div class="uk-button-group" id="buttons" style="position:fixed;bottom:0">
            <button v-for="(button,index) in buttons" @click="$root.sendKey(button.color)" :class="'but-' + button.color" class="uk-button uk-margin-small-top" type="button">{{button.label}}</button>
        </div>
    </div>
</template>
<script>
import o2vEvent from './Event.vue';
import o2vTextmenu from './Textmenu.vue';
import o2vTextarea from './Textarea.vue';
  
function getClearData(){
  return {
      title: '',
      category: -1,
      pageCount:0,
      pageCurrent:0,
      event:{},
      buttons: []
  }
}
export default {
    name: 'o2vOsd',
    data: function() {
        return getClearData();
    },
    created() {
        let menuItem= {
            label: '_O_SD',
            isHidden: function(navComp){
                return !navComp.$root.isActive
            },
            key: 'menu'
        };
        this.$root.$data.menuItems.push(menuItem);
        this.$root.$on("clearmenu", (data) => {
           let clearData= getClearData();
           for (let key in clearData)
                this[key]= clearData[key];
            menuItem.on= false;
        });
        this.$root.$on("menu", (data) => {
            this.category = data.category;
            this.title = data.title;
            menuItem.on= true;
        });
        this.$root.$on("scrollbar", (data) => {
            this.pageCount = parseInt(data.Total / this.$root.maxLines,10);
            this.pageCurrent =  parseInt(data.Offset / this.$root.maxLines,10);
        });
        this.$root.$on("event", (data) => {
            this.event = data;
        });
        this.$root.$on("buttons", (data) => {
            this.buttons = [];
            for (let color in data)
                this.buttons.push({
                    'color': color,
                    label: data[color]
                });
        })
        window.addEventListener('resize', this.checkButtonHeight);
    },
    updated() {
        document.getElementById('o2vContent').style.display= this.title ? "none" : '';
        this.checkButtonHeight();
    },
    methods: {
        checkButtonHeight() {
            let buttons = document.getElementById('buttons');
            if (buttons)
                buttons.parentNode.style.paddingBottom = buttons.offsetHeight + 'px';
        }
    },
    components: {
        'o2v-event': o2vEvent,
        'o2v-textmenu': o2vTextmenu, 
        'o2v-textarea': o2vTextarea
    }
}
</script>
<style>
    button.but-red {
        background-color: red;
        color: #fff;
    }

    button.but-green {
        background-color: green;
        color: #fff;
    }

    button.but-blue {
        background-color: blue;
        color: #fff;
    }

    button.but-yellow {
        background-color: yellow;
        color: #222;
    }
</style>