<template>
  <div v-if="customdata" class="list-group" id="customdata" style="overflow: hidden;">
    <div v-for="(custom,n) in customdata"
         class="mt-2 customdata list-group-item card flex-column p-1 active">
      <div class="d-flex w-100 justify-content-between tmtxt">
        <div>
          <small>{{custom.file}}</small>
          <small>{{custom.name}}: </small>
          <small>{{custom.value}}</small>
        </div>
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
            this.customdata = data && data.length ? data : null;
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
