<template>
    <div v-show="title" id="osdCon">
        <div class="container" @click="$root.sendKey('Back')">
            <h3 class=""><icon v-if="!$root.isOnlyView" name="osd-back"></icon> {{ title }}</h3>
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
        <o2w-textmenu></o2w-textmenu>
        <o2w-event :event="event"></o2w-event>
        <o2w-textarea></o2w-textarea>
        <div class="btn-group fixed-bottom" id="buttons" st__yle="position:fixed;bottom:0">
            <button v-for="(button,index) in buttons" @click="$root.sendKey(button.color)" :class="'but-' + button.color" class="btn btn-primary" type="button">{{button.label}}</button>
        </div>
    </div>
</template>
<script>
require("common").Icon.register({"osd-back":{"width":1280,"height":1792,"paths":[{"d":"M1171 301l-531 531 531 531q19 19 19 45t-19 45l-166 166q-19 19-45 19t-45-19l-742-742q-19-19-19-45t19-45l742-742q19-19 45-19t45 19l166 166q19 19 19 45t-19 45z"}]}})

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
    name: 'o2wOsd',
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
                this.$root.$set(menuItem, "on",false);
        });
        this.$root.$on("menu", (data) => {
            this.category = data.category;
            this.title = data.title;
                this.$root.$set(menuItem, "on",true);
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
        document.getElementById('o2wContent').style.display= this.title ? "none" : '';
        this.checkButtonHeight();
    },
    methods: {
        checkButtonHeight() {
            let buttons = document.getElementById('buttons');
            if (buttons)
                buttons.parentNode.style.paddingBottom = buttons.offsetHeight + 'px';
        }
    }
}
</script>