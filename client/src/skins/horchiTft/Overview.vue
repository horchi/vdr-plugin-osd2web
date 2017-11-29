<template>
  <div id="Overview">
    <div v-if="diashow_active">
      <o2w-diashow :diashow="diashow"></o2w-diashow>
    </div>
    <div v-else="">
      <div class="row dataarea">
        <o2w-actual class="eventarea col-12 col-md-9" />
        <div class="rightarea pr-2 col-12 col-md-3">
          <div class="timerarea">
            <o2w-timer />
          </div>
          <o2w-customdata class="customarea" />
          <o2w-recording class="recordingarea" />
        </div>
      </div>
    </div>
    <o2w-statusbar />
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
