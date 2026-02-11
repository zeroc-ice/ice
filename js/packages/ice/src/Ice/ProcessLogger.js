// Copyright (c) ZeroC, Inc.

import { Logger } from "./Logger.js";

let processLogger = null;

export function getProcessLogger(prefix = "") {
    if (processLogger === null) {
        processLogger = new Logger(prefix);
    }
    return processLogger;
}

export function setProcessLogger(logger) {
    processLogger = logger;
}
