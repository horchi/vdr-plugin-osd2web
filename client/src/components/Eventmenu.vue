<template>
  <table class="table table-hover textmenu" :class="{'text-nowrap':$root.isOnlyView}">
    <tbody>
      <tr v-for="(row, rowIndex) in rows"
          :key="rowIndex"
          :class="{'table-active': rowIndex == textmenucurrent, 'textmenu-current': rowIndex == textmenucurrent}"
          @click="row.selectable && doAction(rowIndex)">
        <template v-if="row.event.eventid > 0">
          <td :key="index" class="menurow">
            <img style="max-height: 40%" :src="'/data/channellogo?name=' + logoname(rowIndex) + '&id=' + row.event.channel.channelid" />
          </td>
          <td class="menurow"> {{ $root.formatTime(row.event.starttime) }} </td>
          <td class="menurow"> {{ row.event.title }} </td>
        </template>
        <template v-else="">
          <td colspan="100%" class="textmenu-sep"> {{ row.event.shorttext }} </td>
        </template>
      </tr>
    </tbody>
  </table>
</template>

<script>

function getClearData() {
    return {
        rows: [],
        textmenucurrent: 0
    }
}
export default {
    name: 'o2wEventmenu',
    data: function() {
        return getClearData();
    },
    created() {
        this.$root.$on("clearmenu", (data) => {
            let clearData= getClearData();
            for (let key in clearData)
                this[key]= clearData[key];
        });
        this.$root.$on("menu", (data) => {

        });
        this.$root.$on("eventitem", (data) => {
            this.$set(this.rows, data.index, {
                event: data.event,
                channelname: data.event.channel ? data.event.channel.channelname : "",
                selectable: data.selectable
            });
            if (data.current)
                this.textmenucurrent = data.index;
        });
    },
    methods: {
        logoname(rowIndex, key) {
            let row = this.rows[rowIndex];
            return this.$root.hasChannelLogos && row.event.channel.channelname ? encodeURIComponent(row.event.channel.channelname) : '';
        },
        doAction(rowIndex, key) {
            let delta = rowIndex - this.textmenucurrent;
            if (delta != 0) {
                let rep=0;
                let row;
                if (delta > 0) {
                    for(delta= this.textmenucurrent; delta < rowIndex; delta++) {
                        row= this.rows[delta];
                        if (row && row.selectable)
                            rep++;
                    }
                    this.$root.sendKey("Down", rep);
                } else {
                    for(delta= rowIndex; delta < this.textmenucurrent; delta++) {
                        row= this.rows[delta];
                        if (row && row.selectable)
                            rep++;
                    }
                    this.$root.sendKey("Up", rep);
                }
            }
            else
                this.$root.sendKey(key || "Ok");
        }
    }
}
</script>
