
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

Eine zusätzliche Entwicklung, die nicht in das osd2web-Plugin einfliessen soll, findet im Ordner
src/custom
statt. Dieser Ordner wird beim git pull ignoriert.
Beispiele, bzw. Vorlagen finden sich im Ordner custom_sample

Dabei entspricht der Ordnername unterhalb von custom dem Skinname.
 - Dieser Ordner muss mindestesens ein theme-file (xxx.scss) haben.
 - Entspricht der Orndername einem vorhanden (build-in) Skin,
   werden die gefunden themes dem Skin hinzugefügt;
   andernfalls muss auch eine  main.js vorhanden sein, damit daraus ein neuer Skin erstellt wird

Um ein zusätzliches Skin oder Theme hinzuzufügen, welches nicht in der Branch ist:
#> cp src/custom_sample/config.js src/custom/    ggf. anpassen

Compilieren der Skins / Themes:
==========================================================

Alle Skins:
#> npm run build
oder ein einzelnes:
#> npm --skin=<the skin> run build
  -> der code wird unter osd2web/client/dist erstellt,
     welcher dann beim "osd2web/make install" (make install-http genügt auch) an die richtige Stelle kopiert wird.
     Wird in der custom/config.js "targetFolder" überschrieben, wird der code natürlich dort erstellt

Praktisches auto-build nach 'dev' während der Entwicklung:
==========================================================

#> npm run dev
für ein spezifisches skin:
#> npm --skin=<the skin> run dev
oder wenn man auch an den allgemeinen code etwas ändern möchte:
#> npm --skin=skin-example --common=1 run dev

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

Möchte man einem vorhanden Skin eine neues Theme hinzufügen:
#> mkdir src/custom/<the skin>
#> cp src/custom_sample/default/theme-example.scss src/custom/<the skin>/<the theme>.scss

Anschlissend kann man das Theme fogendermassen aktivieren:
  http://<server>:<port>/dev/skins/<the skin>/index.html?theme=<the theme>
oder hier auswählen:
  http://<server>:<port>

======================================================
Skin-Entwicklung:
======================================================

#> mkdir src/custom/<the skin>
#> cp src/custom_sample/skin-example/* src/custom/<the skin>/

Um jetzt Veränderungen vorzunehmen, kopiert man sich die zu verändernde(n) Komponente(n) aus
src/components in das Verzeichnis src/custom/<the skin>/ (siehe Beispiel Event.vue)

======================================================
Icons:
======================================================

Hier kann man sich die Icons ansehen:
    http://fontawesome.io/icons/

Wenn in einer Komponente ein Icon verwendet wird, darf dieses NICHT nach Anleitung per
    import 'vue-awesome/icons/xx'
eingebunden werden, weil sonst das skin riesig wird, da das Vue Framework die erneut einbindet.

Stattdessen sollte das so eingebunden werden:
    Aus der vue-awesome/icons/xx.js die Register-Zeile kopieren und die direkt in die Komponenete
    in den Script-Block kopieren und common vor das Icon schreiben:
    require("common").Icon.register({"osd-back":{"width":1280,"height":1792,"paths":[{"d":"M1171 ... 45z"}]}})
