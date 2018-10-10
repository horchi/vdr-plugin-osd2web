<template>
  <div id="app-view" class="viewport">
    <o2w-navigation v-if="!$root.isOnlyView" :items="$root.menuItems" :itemsRight="$root.menuItemsRight"></o2w-navigation>
    <o2w-statusmessage></o2w-statusmessage>
    <div class="d-flex flex-row">
      <o2w-remote v-if="!$root.isOnlyView" />
      <div id="o2wContainer" class="container-fluid">
        <o2w-osd id="osdCon" name="osd" v-show="$root.curView == 'osd'" />
        <o2w-overview id="o2wContent"   v-show="$root.curView != 'osd'" />
      </div>
    </div>
  </div>
</template>

<script>
export default {
    name: 'o2w-app'
}
</script>
