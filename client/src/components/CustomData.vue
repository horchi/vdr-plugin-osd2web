<template>
  <div v-if="customdata" id="customdata" style="overflow: hidden;">
    <div v-for="(item, key, index) in customdata" class="mt-2 w-100 justify-content-between customdata card flex-column p-1 active">
      <small class="customfiletxt">{{key}}</small>
      <div v-for="(variable, n) in item" class="row">
        <small :style="{color: variable.color}" class="col-12 col-sm-8 customname">{{variable.name}}</small>
        <small class="col-12 col-sm-4 customvalue">{{variable.value}}</small>
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
