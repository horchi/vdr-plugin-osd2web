if (!global.Intl) {
    require.ensure([/* webpackChunkName: "intl" */ 
        'intl',
        'intl/locale-data/jsonp/de.js'
    ], function (require) {
        require(/* webpackChunkName: "intl" */ 'intl');
        require('intl/locale-data/jsonp/de.js');
    },"intl");
}

import VueInst from 'vue';

export var Vue = VueInst;
export var Icon = require('vue-awesome/components/Icon');


// Diese Methode wird aufgerufen, um die maximale Anzahl Zeilen für das OSD zu ermitteln.
// Die Werte sollten irgendwie dynamisch ermittelt werden, da sie vom style abhängig sind
export var maxLinesCalc= {
    buttongroupHeight: 29,      // Höhe der Buttonleiste am unteren Rand
    headlineHeight: 38,         // Höhe der Headline des OSD
    lineHeight: 49,             // Höhe einer Zeile

    // Berechnung der maximal Anzahl Zeilen
    getMax: function(){
        return parseInt((window.innerHeight - document.body.firstElementChild.offsetTop - this.headlineHeight - this.buttongroupHeight) / this.lineHeight, 10)
    }
}

VueInst.component('icon', Icon);

global.jQuery = global.$= require('jquery-slim');
global.Popper = require('popper.js');


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


// Einbetten des styles, der in der Url mit ?theme=xxxangegeben werden muss
(function(){
   let theme= location.search.match(/[?&]theme=([^&]+)/);
   let styleEl= document.createElement("link");
   styleEl.setAttribute("rel", "stylesheet");
   styleEl.setAttribute("type", "text/css");
   styleEl.setAttribute("href", 'themes/' + (theme ? theme[1] : 'default') + '.css');
   document.head.appendChild(styleEl);
})();


$(function(){
    if (/[?&]xsnow=1/.test(location.search)) {
        require.ensure([/* webpackChunkName: "xsnow" */ './lib/xsnow/snow'
        ], function (require) {
            var snow= require(/* webpackChunkName: "xsnow" */ './lib/xsnow/snow');
            snow.start(/[?&]santa=1/.test(location.search)); 
        },"xsnow");
    }
})
