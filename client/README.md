*****************************************************************
***                                                           ***
***        Erstellen und ändern von Skins und Themes          ***
***                                                           ***
***  -------------------------------------------------------  ***
***                                                           ***
***  Die Programmierung basiert auf Vue.js                    ***
***     https://vuejs.org/                                    ***
***  Als Responsive Ui-Framework wurde bootstrap 4 benutzt    ***
***     https://v4-alpha.getbootstrap.com/                    ***
***  Die Icons basieren auf vue-awesome                       ***
***     https://github.com/Justineo/vue-awesome               ***
***                                                           ***
*****************************************************************

--------------------------------------------------------------
- Initiale Vorbereitung zum erstellen von Skins und Themes
--------------------------------------------------------------

Benötigt wird nodejs und npm, under Debian basierten Systemen:
#> sudo apt install nodejs

Dieses Paket bringt in der Regel bereist npm mit. 'nodejs' wird mintestens in Version v6.11.1 benötigt.

Wenn es bei Ubuntu klemmt weil ggf das mitgelieferte nodejs zu alt ist:
  - entweder hier: https://www.ubuntuupdates.org/ppa/nodejs_8.x
  - oder hier https://www.digitalocean.com/community/tutorials/how-to-install-node-js-on-ubuntu-16-04
    wie unter "How To Install Using a PPA" beschieben
    das passende PPA für das vorliegende System bzw. Distributuion installieren.

Nun kann das Build-Environment für den Client installiert werden:
#> make buildenv

Somit ist nun alles bereit für die Skin Entwicklung!

Am besten man beginnt nun mit einer Kopie eines der mitgelieferten Skins und liest sich etwas unber die oben
genannten Seiten in 'bootstrap' und 'vue-awesome' ein.

--------------------------------------------------------------
- Entwicklung
--------------------------------------------------------------

Eine zusätzliche Entwicklung, die nicht in das osd2web-Plugin git der Entwickler einfliessen soll,
findet im Ordner src/custom statt. Dieser Ordner wird beim git pull ignoriert.

Beispiele, bzw. Vorlagen finden sich im Ordner custom_sample

Dabei entspricht der Ordnername unterhalb von custom dem Skinname.
 - Dieser Ordner muss mindestesens ein theme-file (xxx.scss) haben.
 - Entspricht der Orndername einem vorhanden (build-in) Skin,
   werden die gefunden themes dem Skin hinzugefügt;
   andernfalls muss auch eine  main.js vorhanden sein, damit daraus ein neuer Skin erstellt wird

Um ein zusätzliches Skin oder Theme hinzuzufügen, welches nicht in der Branch ist:
#> cp src/custom_sample/config.js src/custom/

--------------------------------------------------------------
- Compilieren der Skins / Themes
--------------------------------------------------------------

Alle Skins neu bauen (incl. der in custom) im Plugin Ordner:
#> make cbuild
#> make install

Ein einzelnes Skins neu bauen:
#> npm --skin=<the skin> run build
  -> der code wird unter osd2web/client/dist erstellt,
     welcher dann beim "osd2web/make install" (make install-http genügt auch) an die richtige Stelle kopiert wird.
     Wird in der custom/config.js "targetFolder" überschrieben, wird der code natürlich dort erstellt

--------------------------------------------------------------
- Praktisches auto-build nach 'dev' während der Entwicklung
--------------------------------------------------------------

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

--------------------------------------------------------------
- Theme Entwicklung
--------------------------------------------------------------

Möchte man einem vorhanden Skin eine neues Theme hinzufügen:
#> mkdir src/custom/<the skin>
#> cp src/custom_sample/default/theme-example.scss src/custom/<the skin>/<the theme>.scss

Anschlissend kann man das Theme fogendermassen aktivieren:
  http://<server>:<port>/dev/skins/<the skin>/index.html?theme=<the theme>
oder hier auswählen:
  http://<server>:<port>

--------------------------------------------------------------
- Skin - Entwicklung
--------------------------------------------------------------

#> mkdir src/custom/<the skin>
#> cp src/custom_sample/skin-example/* src/custom/<the skin>/

Um jetzt Veränderungen vorzunehmen, kopiert man sich die zu verändernde(n) Komponente(n) aus
src/components in das Verzeichnis src/custom/<the skin>/ (siehe Beispiel Event.vue)

--------------------------------------------------------------
- Icons
--------------------------------------------------------------

Hier kann man sich die Icons ansehen:
    http://fontawesome.io/icons/

Wenn in einer Komponente ein Icon verwendet wird, darf dieses NICHT nach Anleitung per
    import 'vue-awesome/icons/xx'
eingebunden werden, weil sonst das skin riesig wird, da das Vue Framework die erneut einbindet.

Stattdessen sollte das so eingebunden werden:
    Aus der vue-awesome/icons/xx.js die Register-Zeile kopieren und die direkt in die Komponenete
    in den Script-Block kopieren und common vor das Icon schreiben:
    require("common").Icon.register({"osd-back":{"width":1280,"height":1792,"paths":[{"d":"M1171 ... 45z"}]}})
