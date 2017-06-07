<template>
    <div>
       <div class="row">
           <o2w-actual class="col-12 col-md-6 mb-2" />
           <o2w-timer class="col-12 col-md-6" />
       </div>
       <h4 align="right" class="status-bar">{{ $root.formatDateTime(parseInt(new Date().getTime() / 1000, 10)) }}</h4>
    </div>
</template>
<script>

export default {
    name: 'o2wOverview'
}
</script>