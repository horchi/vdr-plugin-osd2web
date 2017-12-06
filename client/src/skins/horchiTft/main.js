import { root, Vue } from '../../root';

window.v = new Vue({
    mixins: [root]
}).$mount("#app")
