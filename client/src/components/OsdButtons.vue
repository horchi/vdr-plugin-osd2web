<template>
  <div class="btn-group btn-group-sm justify-content-center fixed-bottom osdbuttons" id="buttons">
    <a v-show="pageUp" class="btn btn-secondary" @click="$root.sendKey('Up',$parent.maxLines)"><icon name="caret-up" /></a>
    <button v-for="(button,index) in buttons"
            @click="$root.sendKey(button.color)"
            :class="'but-' + button.color"
            class="btn btn-primary"
            type="button">{{button.label}}</button>
    <a v-show="pageDn" class="btn btn-secondary" @click="$root.sendKey('Down',$parent.maxLines)"><icon name="caret-down" /></a>
  </div>
</template>

<script>

function getClearData() {
    return {
        buttons: [],
        pageUp: false,
        pageDn: false
    }
}

export default {
    name: 'o2wOsdButtons',
    data: function() {
        return getClearData();
    },
    created() {
        this.$root.$on("buttons", (data) => {
            this.buttons = [];
            for (let color in data)
                this.buttons.push({ 'color': color, label: data[color] });
        });
        this.$root.$on("scrollbar", (data) => {
            this.pageUp= data.Offset > 0;
            this.pageDn= (data.Total - this.$parent.maxLines) > data.Offset;
        });
        this.$root.$on("clearmenu", (data) => {
            let clearData= getClearData();
            for (let key in clearData)
                this[key]= clearData[key];
        });
    },
    components: {
    }
}

</script>
