import { root, Vue } from '../../root';
require("common").maxLinesCalc.canScroll= false;

window.v = new Vue({
    mixins: [root]
}).$mount("#app")
