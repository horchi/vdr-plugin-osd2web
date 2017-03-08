<template>
<div id="app-view">
    <o2v-navigation :items="$root.menuItems" :itemsRight="$root.menuItemsRight"></o2v-navigation>
    <o2c-statusmessage></o2c-statusmessage>
    <div class="uk-flex uk-flex-wrap-space-around">
       <o2v-remote></o2v-remote>
       <o2v-osd name="osd"></o2v-osd>
       <div id="o2vContent" class="uk-width-1-1">
          <o2c-actual></o2c-actual>
       </div>
    </div>
</div>
</template>
<script>
import o2vNavigation from './views/Navigation.vue';
import o2vOsd from './views/Textmenu.vue';
import o2vRemote from './views/Remote.vue';
import o2vStatusmessage from './views/Statusmessage.vue';
import o2vActual from './views/Actual.vue';

export default {
    name: 'app',
    components: {
        'o2v-navigation': o2vNavigation,
        'o2v-remote': o2vRemote,
        'o2v-osd': o2vOsd,
        'o2c-statusmessage': o2vStatusmessage,
        'o2c-actual': o2vActual
    }
}
</script>

<style>

</style>
