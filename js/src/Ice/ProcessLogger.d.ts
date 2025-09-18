// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /** Returns the process-wide logger.
         *
         * @returns The process-wide logger.
         */
        function getProcessLogger(): Ice.Logger;

        /** Changes the process-wide logger.
         *
         * @param logger The new process-wide logger.
         */
        function setProcessLogger(logger: Ice.Logger): void;
    }
}
