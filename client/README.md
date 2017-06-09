
*****************************************************************
* Allgemein
*****************************************************************

Die Programmierung basiert auf Vue.js
    https://vuejs.org/
Als Responsive Ui-Framework wurde bootstrap 4 benutzt (nicht vom alpha in der Version verwirren lassen...)
    https://v4-alpha.getbootstrap.com/
Die Icons basieren auf vue-awesome
    https://github.com/Justineo/vue-awesome

*****************************************************************
* Weiterentwicklung und Compilierung
*****************************************************************

Initiale Vorbereitung zum erstellen von Skins / Themes
--------------------------------------------------------------

installieren von npm (ggf. auch node):

sudo apt install npm  (oder wie das bei eurer Distri geht)
cd ... osd2web/client
npm install
    -> dauert etwas ...

Compilieren:

npm run build
  -> der code wird unter osd2web/client/dist erstellt,
     welcher dann beim "osd2web/make install" an die richtige Stelle kopiert wird.

Wenn es bei Ubuntu klemmt weil ggf das mitgelieferte node-js zu alt ist:
 https://www.digitalocean.com/community/tutorials/how-to-install-node-js-on-ubuntu-16-04


*****************************************************************
* Skins / Themes Entwicklung
*****************************************************************

Praktisches auto-build nach 'dev' während der Entwicklung:
=====================================================
npm run dev
  -> alle Änderungen an den Quellen werden überwacht und man kann sich das Ergebniss unter
      http://<server>:<port>/dev/skins/the skin>/index.html anschauen.


======================================================
Theme-Entwicklung:
======================================================

Wenn es sich um ein generelles theme handelt, kopiert man am besten die default.scss:
cp src/themes/default.scss src/skins/<the skin>/themes/myTheme.scss

edit skins.config.js  (ggf. cp skins.config.js.sample skins.config.js) anpassen, bzw. erweitern, zb:

module.exports = {
  'default': ['default', 'myTheme']
};

Anschlissend kann man das Theme fogendermassen aktivieren:
http://<server>:<port>/dev/skins/mySkin/index.html?theme=myTheme
oder hier auswählen:
http://<server>:<port>

======================================================
Skin-Entwicklung:
======================================================

mkdir src/skins/mySkin
cp src/skins/default/main.js src/skins/mySkin/

edit skins.config.js  (ggf. cp skins.config.js.sample skins.config.js) anpassen, bzw. erweitern, zb:

module.exports = {
  'default': ['default'],
  'mySkin': ['default']
};

Um jetzt Veränderungen vorzunehmen, kopiert man sich die zu verändernde(n) Komponente(n) aus
src/components in das Verzeichnis src/skins/mySkin

======================================================
Icons:
======================================================

Hier kann man sich die Icons ansehen:
    http://fontawesome.io/icons/

Wenn in einer Komponente ein Icon verwendet wird, darf dieses NICHT nach Anleitung per
    import 'vue-awesome/icons/xx'
eingebunden werden, weil sonst das skin riesig wird, weil der das Vue Framework erneut einbindet.

Stattdessen sollte das so eingebunden werden:
    Aus der vue-awesome/icons/xx.js die Register-Zeile kopieren und die direkt in die Komponenete
    in den Script-Block kopieren und common vor das Icon schreiben:
    require("common").Icon.register({"osd-back":{"width":1280,"height":1792,"paths":[{"d":"M1171 ... 45z"}]}})
