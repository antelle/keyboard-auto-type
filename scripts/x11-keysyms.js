const https = require('https');
const fs = require('fs');
const path = require('path');

const CACHE_PATH = 'build/keysymdef.h';
const HEADER_URL = 'https://gitlab.freedesktop.org/xorg/proto/xorgproto/-/raw/master/include/X11/keysymdef.h';
const FILE_PATH = path.join(__dirname, '../keyboard-auto-type/src/impl/linux/x11-keysym-map.cpp');

(async function main() {
    try {
        console.log('Building X11 keysyms...');

        const update = process.argv.some(arg => arg.startsWith('--update'));
        const printAll = process.argv.some(arg => arg.startsWith('--print-all'));

        const contents = await loadFile(update);
        const symbolMap = buildSymbolMap(contents);
        const { plus1M, mapped } = symbolMap;

        const special = new Map();
        for (const [codePoint, keySym] of mapped) {
            if (keySym > 0xffff) {
                special.set(codePoint, keySym);
                mapped.delete(codePoint);
            }
        }

        console.log(`Plus1M: ${plus1M.size}, mapped: ${mapped.size}, special: ${special.size}`);
        if (printAll) {
            printSymbolMap(symbolMap);
        }

        const minCodePoint = Math.min(...mapped.keys());
        const maxCodePoint = Math.max(...mapped.keys());
        if (maxCodePoint >= 0xffff) {
            throw new Error(`Too high CodePoint: ${maxCodePoint}`);
        }

        const minKeySym = Math.min(...mapped.values());
        const maxKeySym = Math.max(...mapped.values());
        if (maxKeySym >= 0xffff) {
            throw new Error(`Too high KeySym: ${maxKeySym}`);
        }

        console.log(`CodePoints: 0x${minCodePoint.toString(16).padStart(4, '0')} .. 0x${maxCodePoint.toString(16)}`);
        console.log(`KeySyms: 0x${minKeySym.toString(16).padStart(4, '0')} .. 0x${maxKeySym.toString(16)}`);

        const template = fs.readFileSync(FILE_PATH, 'utf8');
        const code = generateCode(template, mapped, special);
        fs.writeFileSync(FILE_PATH, code);

        console.log(`Done: ${FILE_PATH} updated, don't forget to run "make format"`);
    } catch (e) {
        console.error(e);
    }
}());

async function loadFile(update) {
    if (!update && fs.existsSync(CACHE_PATH)) {
        console.log('Using cached keysymdef.h');
        return fs.readFileSync(CACHE_PATH, 'utf8');
    }

    console.log('Downloading keysymdef.h...');

    return new Promise((resolve, reject) => {
        https.get(HEADER_URL, res => {
            const data = [];
            res.on('data', chunk => data.push(chunk));
            res.on('end', () => {
                const content = Buffer.concat(data);
                fs.writeFileSync(CACHE_PATH, content);
                resolve(content.toString('utf8'));
            });
            res.on('error', e => reject(e));
        });
    });
}

function buildSymbolMap(contents) {
    let plus1M = new Map();
    let mapped = new Map();
    let lineNumber = 0;
    for (const line of contents.split('\n')) {
        lineNumber++;
        if (!line.startsWith('#define')) {
            continue;
        }
        const match = line.match(/^#define\s+(XK_\w+)\s+0x([0-9a-fA-F]+)(\s+\/\*.*\*\/\s*)?$/);
        if (!match) {
            throw new Error(`Bad line :${lineNumber}: ${line}`);
        }
        const [ , , keySymStr, comment] = match;
        const keySym = parseInt(keySymStr, 16);
        if (comment) {
            const codePointMatch = comment.match(/U\+([0-9A-Fa-f]+)/);
            if (codePointMatch) {
                const codePoint = parseInt(codePointMatch[1], 16);
                if (keySym === 0x1_000_000 + codePoint) {
                    if (!plus1M.has(codePoint)) {
                        plus1M.set(codePoint, keySym);
                    }
                } else {
                    if (!mapped.has(codePoint)) {
                        mapped.set(codePoint, keySym);
                    }
                }
            }
        }
    }

    return {
        plus1M,
        mapped
    };
}

function printSymbolMap({ plus1M, mapped }) {
    console.log('Plus1M:');
    console.log([...plus1M.keys()].sort((a, b) => a - b).map(k => k.toString(16)).join('\n'));

    console.log('Mapped:');
    console.log([...mapped.entries()].sort(([a], [b]) => a - b)
        .map(([k, v]) => `${k.toString(16)} => ${v.toString(16)}`).join('\n'));
}

function generateCode(template, mapped, special) {
    const charMap16Code = [...mapped]
        .sort(([cp1], [cp2]) => cp1 - cp2)
        .map(([codePoint, keySym]) => `0x${wordHex(codePoint)}'${wordHex(keySym)}U`)
        .join(', ');
    let result = template.replace(/CHAR_MAP_16\s*\{[\s\S]*?\}/, () => {
        found = true;
        return `CHAR_MAP_16{\n    ${charMap16Code}\n}`;
    });
    if (!found) {
        throw new Error(`Code not found: CHAR_MAP_16{...}`);
    }

    const charMap32Code = [...special]
        .sort(([cp1], [cp2]) => cp1 - cp2)
        .map(([codePoint, keySym]) => `0x${dwordHex(codePoint)}'${dwordHex(keySym)}U`)
        .join(', ');
    result = result.replace(/CHAR_MAP_32\s*\{[\s\S]*?\}/, () => {
        found = true;
        return `CHAR_MAP_32{\n    ${charMap32Code}\n}`;
    });
    if (!found) {
        throw new Error(`Code not found: CHAR_MAP_32{...}`);
    }

    return result;
}

function wordHex(num) {
    if (num <= 0 || num >= 0xffff) {
        throw new Error(`Bad word ${num}`);
    }
    return num.toString(16).padStart(4, '0');
}

function dwordHex(num) {
    if (num <= 0 || num >= 0xffffffff) {
        throw new Error(`Bad dword ${num}`);
    }
    return num.toString(16).padStart(8, '0');
}
