<template>
    <table v-if="colCount > 0" class="uk-table uk-table-hover" :class="{'uk-text-nowrap':$root.isOnlyView}">
        <tbody>
            <tr v-for="(row, rowIndex) in rows" :key="rowIndex" :class="{'uk-form-success': rowIndex == textmenucurrent}" @click="row.selectable && doAction(rowIndex)">
                <td v-for="(col, index) in row.cols" :key="index" :class="{'uk-placeholder': rowIndex == textmenucurrent && index== 1 && canEdit == 1}">{{col}}</td>
                <td v-if="row.selectable && canEdit==1" class="uk-button-group">
                    <a class="uk-button" @click.stop="doAction(rowIndex,'Left')"><i uk-icon="icon:triangle-left"></i></a>
                    <a v-show="row.textEdit" class="uk-button" @click.stop="$root.sendKey('Up')"><i uk-icon="icon:triangle-up"></i></a>
                    <a v-show="row.textEdit" class="uk-button" @click.stop="$root.sendKey('Down')"><i uk-icon="icon:triangle-down"></i></a>
                    <a class="uk-button" @click.stop="doAction(rowIndex,'Right')"><i uk-icon="icon:triangle-right"></i></a>
                </td>
            </tr>
        </tbody>
    </table>
</template>

<script>

function getClearData(){
  return {
      rows: [],
      colCount: 0,
      textmenucurrent: 0,
      canEdit: 0,
  }
}
export default {
    name: 'o2vTextmenu',
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
            if (this.colCount < 0) {
                this.colCount = data.text.split('\t').length;
                if (this.colCount == 2 && this.canEdit)
                    this.colCount++;
                else
                    this.canEdit = 0;
            }
            this.$set(this.rows, data.index, {
                cols: data.text.split('\t'),
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
