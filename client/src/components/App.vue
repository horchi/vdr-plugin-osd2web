<template>
<div id="app-view" class="viewport">
    <o2w-navigation v-if="!$root.isOnlyView" :items="$root.menuItems" :itemsRight="$root.menuItemsRight"></o2w-navigation>
    <o2w-statusmessage></o2w-statusmessage>
    <div class="d-flex flex-row">
       <o2w-remote v-if="!$root.isOnlyView"></o2w-remote>
       <div id="o2wContainer" class="container-fluid">
          <o2w-osd name="osd"></o2w-osd>
          <div id="o2wContent" v-show="!$root.osdOn">
             <o2w-overview></o2w-overview>
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
