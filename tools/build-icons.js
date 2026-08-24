// ============================================================================
//  Erzeugt include/icons.h aus den PNG-Dateien in tools/icons/.
//
//  Die Home-Screen-Icons der installierbaren App (PWA) werden als Byte-Arrays
//  ins Flash eingebettet und von main.cpp unter festen Routen ausgeliefert:
//    icon-180.png -> /apple-touch-icon.png  (iOS-Home-Screen; iOS akzeptiert
//                                             fuer das App-Icon nur PNG, kein SVG)
//    icon-192.png -> /icon-192.png          (Android/Manifest, Mindestgroesse)
//    icon-512.png -> /icon-512.png          (Android/Manifest, Splash)
//
//  Die PNGs entstehen aus src/web_page.h (icon_svg), gerastert per Browser-
//  Canvas. Wird das Icon geaendert, die PNGs neu rastern und dieses Skript
//  erneut ausfuehren.
//
//  Start:  node tools/build-icons.js   ->  erzeugt include/icons.h
// ============================================================================
const fs = require('fs');
const path = require('path');

const ICON_DIR = path.join(__dirname, 'icons');
const OUT = path.join(__dirname, '..', 'include', 'icons.h');

// PNG-Datei -> C-Symbolname
const ICONS = [
    { file: 'icon-180.png', name: 'icon_180_png' },
    { file: 'icon-192.png', name: 'icon_192_png' },
    { file: 'icon-512.png', name: 'icon_512_png' },
];

function toByteArray(name, buffer) {
    let out = 'const uint8_t ' + name + '[] PROGMEM = {\n';
    for (let i = 0; i < buffer.length; i++) {
        if (i % 16 === 0) out += '    ';
        out += '0x' + buffer[i].toString(16).padStart(2, '0') + ',';
        out += (i % 16 === 15) ? '\n' : ' ';
    }
    if (buffer.length % 16 !== 0) out += '\n';
    out += '};\n';
    out += 'const size_t ' + name + '_len = ' + buffer.length + ';\n';
    return out;
}

let header = '#pragma once\n\n';
header += '// AUTOMATISCH ERZEUGT durch tools/build-icons.js aus tools/icons/*.png.\n';
header += '// Nicht von Hand editieren. Siehe tools/build-icons.js.\n\n';
header += '#include <Arduino.h>\n\n';

for (const icon of ICONS) {
    const buffer = fs.readFileSync(path.join(ICON_DIR, icon.file));
    header += '// ' + icon.file + ' (' + buffer.length + ' Bytes)\n';
    header += toByteArray(icon.name, buffer);
    header += '\n';
}

fs.writeFileSync(OUT, header);
console.log('icons.h erzeugt:', OUT);
