<template>
    <table v-if="colCount > 0" class="table table-hover textmenu" :class="{'text-nowrap':$root.isOnlyView}">
        <tbody>
            <tr v-for="(row, rowIndex) in rows" :key="rowIndex" :class="{'table-active': rowIndex == textmenucurrent, 'textmenu-current': rowIndex == textmenucurrent}" @click="row.selectable && doAction(rowIndex)">
                <td v-for="(col, index) in row.cols" :key="index" :class="{'bg-warning': rowIndex == textmenucurrent && index== 1 && canEdit == 1}">{{col}}</td>
                <td v-if="row.selectable && canEdit==1" class="btn-group btn-group-sm">
                    <a class="btn btn-secondary" @click.stop="doAction(rowIndex,'Left')" ><icon name="caret-left" /></a>
                    <a v-show="row.textEdit" class="btn btn-secondary" @click.stop="$root.sendKey('Up')"><icon name="caret-up" /></a>
                    <a v-show="row.textEdit" class="btn btn-secondary" @click.stop="$root.sendKey('Down')"><icon name="caret-down" /></a>
                    <a class="btn btn-secondary" @click.stop="doAction(rowIndex,'Right')"><icon name="caret-right" /></a>
                </td>
            </tr>
        </tbody>
    </table>
</template>

<script>
var Icon= require("common").Icon;
Icon.register({"caret-left":{"width":640,"height":1792,"paths":[{"d":"M640 448v896q0 26-19 45t-45 19-45-19l-448-448q-19-19-19-45t19-45l448-448q19-19 45-19t45 19 19 45z"}]}})
Icon.register({"caret-right":{"width":640,"height":1792,"paths":[{"d":"M576 896q0 26-19 45l-448 448q-19 19-45 19t-45-19-19-45v-896q0-26 19-45t45-19 45 19l448 448q19 19 19 45z"}]}})
Icon.register({"caret-down":{"width":1024,"height":1792,"paths":[{"d":"M1024 704q0 26-19 45l-448 448q-19 19-45 19t-45-19l-448-448q-19-19-19-45t19-45 45-19h896q26 0 45 19t19 45z"}]}})
Icon.register({"caret-up":{"width":1024,"height":1792,"paths":[{"d":"M1024 1216q0 26-19 45t-45 19h-896q-26 0-45-19t-19-45 19-45l448-448q19-19 45-19t45 19l448 448q19 19 19 45z"}]}})

function getClearData(){
  return {
      rows: [],
      colCount: 0,
      textmenucurrent: 0,
      canEdit: 0,
  }
}
export default {
    name: 'o2wTextmenu',
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
            this.canEdit = data.editable || 0;
            this.colCount= -1;
        });
        this.$root.$on("menuitem", (data) => {
            if (this.colCount < 0 && data.text) {
                this.colCount = data.text.split('\t').length;
                if (this.colCount == 2 && this.canEdit)
                    this.colCount++;
                else
                    this.canEdit = 0;
            }
            this.$set(this.rows, data.index, {
                cols: data.text ? data.text.split('\t') : new Array(this.colCount),
                selectable: data.selectable,
                textEdit: this.canEdit && /.+\\t.*\[.\]/.test(data.text)
            });
            if (data.current)
                this.textmenucurrent = data.index;
        });
    },
    methods: {
        doAction(rowIndex, key) {
            let delta = rowIndex - this.textmenucurrent;
            if (delta != 0){
               let rep=0;
               let row;
               if (delta > 0){
                  for(delta= this.textmenucurrent; delta < rowIndex; delta++){
                     row= this.rows[delta];
                     if (row && row.selectable)
                        rep++;
                  }
                  this.$root.sendKey("Down", rep);
               } else {
                  for(delta= rowIndex; delta < this.textmenucurrent; delta++){
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
