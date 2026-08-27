// ============================================================================
//  Fuegt die Web-App aus web/ zu src/web_page.h zusammen.
//
//  Quellen (getrennt, damit uebersichtlich):
//    web/index.html    HTML + eingebettetes JavaScript (referenziert style.css)
//    web/style.css     das CSS  ->  wird als eigene Seite /style.css ausgeliefert
//    web/manifest.json PWA-Manifest
//    web/icon.svg      App-Icon (SVG)
//
//  Das CSS wird NICHT ins HTML eingebettet: die Seite verlinkt style.css, und
//  der ESP liefert sie unter /style.css aus (siehe handleStyle in main.cpp).
//  So greifen die ESP-Seite und die Demo auf DIESELBE Style-Datei zu.
//
//  Nach Aenderungen in web/ starten:
//    node tools/build-web.js      ->  erzeugt src/web_page.h
//    node tools/build-demo.js     ->  aktualisiert die Demo (+ src/style.css)
// ============================================================================
const fs = require('fs');
const path = require('path');

const WEB = path.join(__dirname, '..', 'web');
const OUT = path.join(__dirname, '..', 'src', 'web_page.h');

const html     = fs.readFileSync(path.join(WEB, 'index.html'), 'utf8').replace(/\s+$/, '');
const css      = fs.readFileSync(path.join(WEB, 'style.css'), 'utf8').replace(/\s+$/, '');
const manifest = fs.readFileSync(path.join(WEB, 'manifest.json'), 'utf8').replace(/\s+$/, '');
const icon     = fs.readFileSync(path.join(WEB, 'icon.svg'), 'utf8').replace(/\s+$/, '');

if (!/<link rel="stylesheet" href="style\.css">/.test(html)) {
    console.error('Fehler: <link ... style.css> in web/index.html nicht gefunden.');
    process.exit(1);
}

const out =
`#pragma once

// AUTOMATISCH ERZEUGT aus web/ durch tools/build-web.js. Nicht von Hand
// editieren - stattdessen web/index.html, web/style.css, web/manifest.json
// oder web/icon.svg aendern und "node tools/build-web.js" erneut ausfuehren.

#include <Arduino.h>

const char index_html[] PROGMEM = R"rawliteral(
${html}
)rawliteral";

const char style_css[] PROGMEM = R"rawliteral(
${css}
)rawliteral";

const char manifest_json[] PROGMEM = R"rawliteral(
${manifest}
)rawliteral";

const char icon_svg[] PROGMEM = R"rawliteral(
${icon}
)rawliteral";
`;

fs.writeFileSync(OUT, out);
console.log('src/web_page.h erzeugt aus web/ (CSS als eigene /style.css).');
