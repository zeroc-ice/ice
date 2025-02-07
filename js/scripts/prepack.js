import fs from 'fs';
import os from 'os';
import path from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

let platformPrefix = '';
let archPrefix = '';
let slice2js = '';
switch (os.platform()) {
    case 'win32':
        platformPrefix = 'windows';
        archPrefix = 'x64';
        slice2js = 'slice2js.exe';
        break;
    case 'darwin':
        platformPrefix = 'macos';
        archPrefix = 'arm64';
        slice2js = 'slice2js';
        break;
    case 'linux':
        platformPrefix = 'linux';
        archPrefix = os.arch();
        slice2js = 'slice2js';
        break;
    default:
        console.error(`Unsupported platform: ${os.platform()}`);
        process.exit(1);
}

const sourceDir = path.resolve(__dirname, '../../slice');
const sliceDestDir = path.resolve(__dirname, '../slice');
let binDestDir = path.resolve(__dirname, '../bin');

function copyFolderSync(src, dest) {
    if (fs.existsSync(dest)) {
        fs.rmSync(dest, { recursive: true, force: true });
    }

    fs.mkdirSync(dest, { recursive: true });

    fs.readdirSync(src).forEach((file) => {
        const srcFile = path.join(src, file);
        const destFile = path.join(dest, file);

        if (fs.lstatSync(srcFile).isDirectory()) {
            copyFolderSync(srcFile, destFile);
        } else {
            fs.copyFileSync(srcFile, destFile);
        }
    });
}

console.log(`Copying Slice files from ${sourceDir} to ${sliceDestDir}...`);
copyFolderSync(sourceDir, sliceDestDir);
console.log('Done.');

const stagingPath = process.env.SLICE2JS_STAGING_PATH;
if (stagingPath) {
    console.log(`Copying Slice compilers from ${sourceDir} to ${binDestDir}...`);
    copyFolderSync(sourceDir, binDestDir);
    console.log('Done.');
} else {
    console.log(`Compiling Slice compiler from cpp build...`);
    let binDir = path.resolve(__dirname, '../../cpp/bin');
    if (os.platform() === 'win32') {
        binDir = path.resolve(binDir, process.env.PLATFORM || "x64", process.env.CONFIGURATION || "Release");
    }
    fs.mkdirSync(path.join(binDestDir, `${platformPrefix}-${archPrefix}`), { recursive: true });
    fs.copyFileSync(path.join(binDir, slice2js), path.join(binDestDir, `${platformPrefix}-${archPrefix}`, slice2js));
}
