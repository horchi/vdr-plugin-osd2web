import { root, Vue } from '../../root';

window.v = new Vue({
    mixins: [root]
}).$mount("#app")

import xsnow from "../../lib/xsnow/snow";
// new xsnow().start();
