import VueInst from 'vue';

export var Vue = VueInst;
export var Icon = require('vue-awesome/components/Icon');

VueInst.component('icon', Icon);

global.jQuery = require('jquery-slim');
global.Tether = require('tether');

require('bootstrap');
/* Einzelne Komponenten von bootstrap:
import 'bootstrap/js/src/alert';
import 'bootstrap/js/src/button';
import 'bootstrap/js/src/carousel';
import 'bootstrap/js/src/collapse';
import 'bootstrap/js/src/dropdown';
import 'bootstrap/js/src/modal';
import 'bootstrap/js/src/popover';
import 'bootstrap/js/src/scrollspy';
import 'bootstrap/js/src/tab';
import 'bootstrap/js/src/tooltip';
*/
(function(){
   let theme= location.search.match(/[?&]theme=([^&]+)/);
   let styleEl= document.createElement("link");
   styleEl.setAttribute("rel", "stylesheet");
   styleEl.setAttribute("type", "text/css");
   styleEl.setAttribute("href", 'themes/' + (theme ? theme[1] : 'default') + '.css');
   document.head.appendChild(styleEl);
})();
