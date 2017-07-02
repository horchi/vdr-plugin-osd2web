<template>
  <div>
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
</template>

<script>

export default {
    name: 'o2wOverview'
}

</script>
