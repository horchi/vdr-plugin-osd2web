<template>
<nav class="uk-navbar">
    <a @click="show_level0= !show_level0" class="uk-navbar-toggle uk-visible-small"></a>
    <ul class="uk-navbar-nav" :class="{'uk-hidden-small':!show_level0}">
        <li v-for="item in items" @click="handleSelect(item)" :class="{'uk-active':item.on}">
            <a v-html="renderLabel(item)"></a>
        </li>
    </ul>
</nav>
</template>

<script>
export default {
    name: 'o2vNavigation',
    props: {
      show_level0: true,
      'items': Array,
    },
    methods: {
        handleSelect(item) {
            if (item) {
                if (item.func)
                    item.func.call(item, this);
                else if (item.key)
                    this.$root.sendKey(item.key);
            }
        },
        renderLabel(item) {
            let key = null;
            let label = item.label.replace(/_(.)_/, function(a, k) {
                key = k.toLowerCase();
                return '<u>' + k + '</u>';
            });
            if (key)
                this.keys[key] = item;
            return (item.iconClass ? '<i class="' + item.iconClass + '"></i>' : '') + label;
        }
    },
    beforeUpdate() {
        //this.keys = {};
    },
    created() {
        this.keys = {};
        window.addEventListener('keyup', (ev) => {
           console.log(this.keys)
            if (ev.altKey && this.keys[ev.key]) {
                this.handleSelect(this.keys[ev.key]);
                ev.stopPropagation()
            }
        });
    }
}
</script>
