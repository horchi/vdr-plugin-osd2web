<template>
    <div>
       <div class="row dataarea">
           <o2w-actual class="eventarea col-12 col-md-9" />
           <o2w-timer class="timerarea col-12 col-md-3" />
       </div>
       <div class="card statusbar">
           <h4 align="right" class="my-auto mr-2">{{ $root.formatDateTime(parseInt(new Date().getTime() / 1000, 10)) }}</h4>
       </div>
    </div>
</template>
<script>

export default {
    name: 'o2wOverview'
}
</script>
