// Copyright (c) ZeroC, Inc.

import { Logger } from "./Logger.js";

let processLogger = null;

export function getProcessLogger() {
    if (processLogger === null) {
        //
        // TODO: Would be nice to be able to use process name as prefix by default.
        //
        processLogger = new Logger("", "");
    }

    return processLogger;
}

export function setProcessLogger(logger) {
    processLogger = logger;
}
