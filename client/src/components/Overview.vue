<template>
  <div id="Overview">
    <div v-if="diashow_active">
      <div class="row diaarea">
        <o2w-diashow :diashow="diashow"></o2w-diashow>
      </div>
    </div>
    <div v-else="">
      <div class="row dataarea">
        <o2w-actual class="eventarea col-12 col-md-9" />
        <div class="rightarea pr-2 col-12 col-md-3">
          <o2w-timer class="timerarea" />
          <o2w-customdata class="customarea" />
          <o2w-recording class="recordingarea" />
        </div>
      </div>
      <o2w-statusbar />
    </div>
  </div>
</template>

<script>

export default {
    name: 'o2wOverview',
    data() {
        return {
            diashow_active: null,
            diashow: {}
        }
    },
    created() {
        this.$root.$on("diashow", (data) => {
            this.diashow_active = data.active;
            this.diashow = data;
        });
    },
    computed: {
    }
}

</script>
