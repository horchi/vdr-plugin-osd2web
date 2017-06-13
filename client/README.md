
*****************************************************************
* Allgemein / Vorbereitung
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

#> sudo apt install npm    # oder wie das bei eurer Distri geht
#> cd ... osd2web/client
#> npm install             # dauert etwas ...

Wenn es bei Ubuntu klemmt weil ggf das mitgelieferte node-js zu alt ist:
  - https://www.digitalocean.com/community/tutorials/how-to-install-node-js-on-ubuntu-16-04
    hier ging es mit dem dort unter "How To Install Using a PPA" beschiebenen problemlos

*****************************************************************
* Skins / Themes Entwicklung
*****************************************************************

Compilieren der Skins / Themes:
==========================================================

#> npm run build
  -> der code wird unter osd2web/client/dist erstellt,
     welcher dann beim "osd2web/make install" (make install-http genügt auch) an die richtige Stelle kopiert wird.

Praktisches auto-build nach 'dev' während der Entwicklung:
==========================================================
Achtung das auto-build funktioniert noch nicht zuverlässig wir sind noch an einer Umstellung der Ordner Struktur dafür.
Bis dahin bitte wie oben beschriben "npm run build" verwenden!

#> npm run dev

  -> alle Änderungen an den Quellen werden überwacht und es wird automatisch nach 'dev' gebaut
   - man kann sich das Ergebniss unter:
      http://<server>:<port>/dev/skins/<the skin>/index.html anschauen.
   - oder mit Angabe des Themes:
      http://<server>:<port>/dev/skins/<the skin>/index.html?theme=<the theme>
   - und onlyView:
      http://<server>:<port>/dev/skins/<the skin>/index.html?onlyView=yes&theme=<the theme>

======================================================
Theme-Entwicklung:
======================================================

Wenn es sich um ein generelles theme handelt, kopiert man am besten die default.scss:
#> cp src/themes/default.scss src/skins/<the skin>/themes/myTheme.scss

#> edit skins.config.js  # (ggf. cp skins.config.js.sample skins.config.js) anpassen, bzw. erweitern, zb:

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
#> cp src/skins/default/main.js src/skins/mySkin/

#> edit skins.config.js  (ggf. cp skins.config.js.sample skins.config.js) anpassen, bzw. erweitern, zb:

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
