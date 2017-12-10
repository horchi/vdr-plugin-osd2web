<template>
  <div v-if="customdata" id="customdata" style="overflow: hidden;">
    <div v-for="(item, key, index) in customdata" class="customdata mt-1 card">
      <div class="customtitle">{{key}}</div>
      <div v-for="(variable, n) in item" class="customrow">
        <div :style="{color: variable.color}" class="customname">{{variable.name}}</div>
        <div class="customvalue">{{variable.value}}</div>
      </div>
    </div>
  </div>
</template>

<script>

export default {
    name: 'o2wCustomData',
    data: function () {
        return {
            customdata: null
        };
    },
    created() {
        this.$root.$on("customdata", (data) => {
            this.customdata = data;
        });
    },
    components: {
    }
}
/*
{
   "event" : "customdata",
   "object" : [
      {
         "file" : "Heizung.data",
         "name" : "Status",
         "value" : "Heizen"
      },
      {
         "value" : "11°C",
         "name" : "Temperatur",
         "file" : "Heizung.data"
      },
      {
         "file" : "FooBar.data",
         "name" : "test",
         "value" : "foo"
      },
      {
         "file" : "FooBar.data",
         "value" : "bar",
         "name" : "test33"
      }
   ]
}
*/
</script>
