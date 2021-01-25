const fs = require('fs');
const path = require('path');
const { app, nativeTheme, ipcMain, BrowserWindow } = require('electron');

let file = fs.createWriteStream(path.join(__dirname, '..', 'build', 'test.txt'));

app.whenReady().then(() => {
    const win = new BrowserWindow({
        width: 600,
        height: 400,
        backgroundColor: nativeTheme.shouldUseDarkColors ? '#242424' : '#ffffff',
        webPreferences: {
            nodeIntegration: false,
            contextIsolation: true,
            preload: path.join(__dirname, 'preload.js')
        }
    });
    win.loadFile('index.html');
});
app.on('window-all-closed', () => { quit(); });

ipcMain.handle('log', async (e, ...args) => {
    file.write(args.join(' ') + '\n');
    console.log(...args);
});
ipcMain.handle('quit', async () => {
    quit();
});

function quit() {
    if (file) {
        file.end('done\n', () => {
            app.quit();
        });
        file = null;
    }
}

setTimeout(() => app.quit(), 30_000);
