// Copyright (c) ZeroC, Inc.

import { spawn } from 'child_process';
import path from 'path';
import os from 'os';
import { fileURLToPath } from 'url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));

let platformPrefix = '';
let archPrefix = '';
let exe = '';
switch (os.platform()) {
    case 'win32':
        platformPrefix = 'windows';
        archPrefix = 'x64';
        exe = 'slice2js.exe';
        break;
    case 'darwin':
        platformPrefix = 'macos';
        archPrefix = 'arm64';
        exe = 'slice2js';
        break;
    case 'linux':
        platformPrefix = 'linux';
        archPrefix = os.arch();
        exe = 'slice2js';
        break;
    default:
        console.error(`Unsupported platform: ${os.platform()}`);
        process.exit(1);
}

const sliceDir = path.resolve(path.join(__dirname, '..', 'slice'));
const binDir = path.resolve(path.join(__dirname, '..', 'bin', `${platformPrefix}-${archPrefix}`));
const slice2js = path.resolve(path.join(binDir, exe));

export function compile(args = [], options) {
    let slice2jsArgs = args.slice();
    slice2jsArgs.push(`-I${sliceDir}`);
    return spawn(slice2js, slice2jsArgs, options);
}

export { sliceDir, slice2js };
