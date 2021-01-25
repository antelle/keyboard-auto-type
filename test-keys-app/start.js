const fs = require('fs');
const path = require('path');
const { spawnSync, spawn } = require('child_process');

if (!fs.existsSync(path.join(__dirname, 'node_modules', 'electron'))) {
    const p = spawnSync('npm', ['ci'], { cwd: __dirname });
    console.log(p.stdout.toString());
}

const ps = spawn('npm', ['start'], { cwd: __dirname, detached: true });
ps.unref();

process.exit(0);
