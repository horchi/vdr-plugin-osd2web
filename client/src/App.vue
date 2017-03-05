<template>
<div id="app-view">
    <o2v-navigation :items="$root.menuItems"></o2v-navigation>
    <o2c-Statusmessage></o2c-Statusmessage>
    <div class="uk-flex uk-flex-wrap-space-around">
        <o2v-remote></o2v-remote>
        <keep-alive>
            <o2v-osd name="osd"></o2v-osd>
        </keep-alive>
    </div>
</div>
</template>
<script>
import o2vNavigation from './views/Navigation.vue';
import o2vOsd from './views/Textmenu.vue';
import o2vRemote from './views/Remote.vue';
import o2vStatusmessage from './views/Statusmessage.vue';
export default {
    name: 'app',
    components: {
        'o2v-navigation': o2vNavigation,
        'o2v-remote': o2vRemote,
        'o2v-osd': o2vOsd,
        'o2c-Statusmessage': o2vStatusmessage
    }
}
</script>

<style>

</style>
