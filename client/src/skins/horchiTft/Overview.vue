<template>
  <div>
    <div class="row dataarea">
      <o2w-actual class="eventarea col-12 col-md-9" />
      <o2w-timer class="timerarea col-12 col-md-3" />
    </div>
    <div class="card statusbar">
      <div class="row my-auto ml-auto" align="right">
        <div class="my-auto mr-3 sdatetxt">{{ $root.formatDateLong(parseInt(new Date().getTime() / 1000, 10))}}</div>
        <div class="my-auto mr-4 stimetxt">{{actualtime}}</div>
      </div>
    </div>
  </div>
</template>
<script>

export default {
    name: 'o2wOverview',
    data(){
        return {
            now: parseInt(new Date().getTime() / 1000, 10)
        }
    },
    computed: {
        actualtime: function () {
            window.setTimeout(()=>{
                this.now= parseInt(new Date().getTime() / 1000, 10);
            },1000);
            return this.$root.formatTimeLong(this.now);
        }
    }
}
</script>
