const fs = require('fs');
const path = require('path');
const { spawnSync, spawn } = require('child_process');

if (!fs.existsSync(path.join(__dirname, 'node_modules', 'electron'))) {
    spawnSync('npm', ['ci'], { cwd: __dirname });
}

const electron = require('electron');
const ps = spawn(electron, ['.'], { cwd: __dirname, detached: true });
ps.unref();

process.exit(0);
