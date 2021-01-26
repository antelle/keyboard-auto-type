const { ipcRenderer } = require('electron');

const CodeMap = {
    // "`" is actually "IntlBackslash" on macOS, but not always:
    // https://www.w3.org/TR/uievents-code/#keyboard-102
    IntlBackslash: 'Backquote'
};

window.addEventListener('DOMContentLoaded', () => {
    const [textarea] = document.getElementsByTagName('textarea');
    for (const evt of ['keypress', 'keyup', 'keydown']) {
        textarea.addEventListener(evt, (e) => {
            const modifiers = ['shift', 'alt', 'ctrl', 'meta']
                .filter(mod => e[`${mod}Key`])
                .join('+') || '-';
            const location = ['standard', 'left', 'right', 'numpad']
                .filter(loc => e.location === KeyboardEvent[`DOM_KEY_LOCATION_${loc.toUpperCase()}`])
                .join('');
            const code = CodeMap[e.code] || e.code;
            ipcRenderer.invoke('log',
                'event',
                e.type,
                e.charCode,
                // e.keyCode,
                modifiers,
                // e.key,
                code,
                location
            );

            if (e.code === 'Tab' && !e.metaKey && !e.ctrlKey && !e.altKey && !e.shiftKey) {
                e.preventDefault();
                if (e.type === 'keydown') {
                    textarea.value += '\t';
                }
                textarea.focus();
            }

            if (e.type === 'keydown' && e.code === 'KeyS' && (e.metaKey || e.ctrlKey)) {
                const text = [...textarea.value].map(ch => ch.codePointAt(0)).join(' ');
                ipcRenderer.invoke('log', 'text', text);
                ipcRenderer.invoke('quit');
            }
        });
    }
    textarea.focus();
    ipcRenderer.invoke('log', 'started');
});
