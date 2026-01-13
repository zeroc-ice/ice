// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Represents Ice's abstraction for logging and tracing. Applications can provide their own logger by
         * implementing this abstraction and setting a logger on the communicator.
         * @see {@link InitializationData}
         */
        interface Logger {
            /**
             * Prints a message. The message is printed literally, without any decorations such as executable name or
             * timestamp.
             * @param message The message to log.
             */
            print(message: string): void;

            /**
             * Logs a trace message.
             * @param category The trace category.
             * @param message The trace message to log.
             */
            trace(category: string, message: string): void;

            /**
             * Logs a warning message.
             * @param message The warning message to log.
             * @see #error
             */
            warning(message: string): void;

            /**
             * Logs an error message.
             * @param message The error message to log.
             * @see #warning
             */
            error(message: string): void;

            /**
             * Returns this logger's prefix.
             * @returns The prefix.
             */
            getPrefix(): string;

            /**
             * Returns a clone of the logger with a new prefix.
             * @param prefix The new prefix for the logger.
             * @returns A new logger instance with the specified prefix.
             */
            cloneWithPrefix(prefix: string): Ice.Logger;
        }
    }
}
