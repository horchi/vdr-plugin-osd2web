<template>
  <div>
    <div class="row">
      <o2w-actual class="col-12 col-md-9 mb-2" />
      <o2w-timer class="col-12 col-md-3" />
    </div>
    <div class="card status-bar">
      <h4 align="right" class="mt-1 mr-2">{{ $root.formatDateTime(parseInt(new Date().getTime() / 1000, 10))}}</h4>
    </div>
  </div>
</template>
<script>

export default {
    name: 'o2wOverview'
}
</script>
