<template>
  <div class="statusbar flexrow">
    <div class="my-auto osdbuttons flexitemgrow" id="buttons">
      <a v-show="pageUp" class="btn btn-secondary" @click="$root.sendKey('Up',maxLines)"><icon name="caret-up" /></a>
      <button v-for="(button,index) in buttons"
              @click="$root.sendKey(button.color)"
              :class="'but-' + button.color"
              class="btn ml-2 btn-primary"
              type="button">{{button.label}}</button>
      <a v-show="pageDn" class="btn btn-secondary" @click="$root.sendKey('Down',maxLines)"><icon name="caret-down" /></a>
    </div>
    <div class="my-auto sdatetxt flexitem">{{ $root.formatDateLong(parseInt(new Date().getTime() / 1000, 10))}}</div>
    <div class="my-auto stimetxt flexitem">{{actualtime}}</div>
  </div>
</template>

<script>

export default {
    name: 'o2wStatusBar',
    data: function() {
        return  {
           now: parseInt(new Date().getTime() / 1000, 10),
           buttons: [],
           pageUp: false,
           pageDn: false
        };
    },
    created() {
        this.$root.$on("buttons", (data) => {
            this.buttons = [];
            for (let color in data)
                this.buttons.push({ 'color': color, label: data[color] });
        });
        this.$root.$on("scrollbar", (data) => {
            this.pageUp= data.Offset > 0;
            this.pageDn= (data.Total - maxLines) > data.Offset;
        });
    },
    computed: {
        actualtime: function () {
            window.setTimeout(() => {
                this.now= parseInt(new Date().getTime() / 1000, 10);
            },1000);
            return this.$root.formatTimeLong(this.now);
        }
    }
}

</script>
