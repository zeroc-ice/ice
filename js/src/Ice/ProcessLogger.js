// Copyright (c) ZeroC, Inc.

let processLogger = null;

export function getProcessLogger(prefix) {
    DEV: console.assert(processLogger !== null);
    return processLogger;
}

export function setProcessLogger(logger) {
    processLogger = logger;
}
