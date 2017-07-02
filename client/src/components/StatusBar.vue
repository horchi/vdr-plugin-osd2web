<template>
  <div class="card statusbar">
    <h4 align="right" class="my-auto mr-2">{{ $root.formatDateTime(parseInt(new Date().getTime() / 1000, 10)) }}</h4>
  </div>
</template>

<script>

export default {
    name: 'o2wStatusBar',
    data: function () {
    },
    created() {
    },
    components: {
    }
}

</script>
