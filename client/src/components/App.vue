<template>
  <div id="app-view" class="viewport">
    <o2w-navigation v-if="!$root.isOnlyView && !$root.isOsdOnly" :items="$root.menuItems" :itemsRight="$root.menuItemsRight"></o2w-navigation>
    <o2w-statusmessage></o2w-statusmessage>
    <div class="d-flex flex-row">
      <o2w-remote v-if="!$root.isOnlyView && !$root.isOsdOnly" />
      <div id="o2wContainer" class="container-fluid">
        <o2w-osd id="osdCon" name="osd" v-show="$root.curView == 'osd'" />
        <o2w-overview id="o2wContent"   v-show="$root.curView != 'osd' && !$root.isOsdOnly" />
        <div v-if="$root.isOsdOnly && $root.curView != 'osd'" class="o2wNoOsd p-3 text-center">
          <p>kein OSD geöffnet</p>
          <button type="button" class="btn btn-primary" @click="$root.sendKey('Menu')">Menü öffnen</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
export default {
    name: 'o2w-app'
}
</script>
