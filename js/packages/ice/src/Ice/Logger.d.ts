// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Represents Ice's abstraction for logging and tracing. Applications can provide their own logger by
         * implementing this abstraction and supplying a logger in {@link InitializationData}.
         * @remarks The Ice runtime calls {@link print}, {@link trace}, {@link warning} and {@link error} while
         * performing internal operations. Implementations of these methods must not throw exceptions: a thrown
         * exception can leave the Ice runtime in an inconsistent state or terminate the program.
         * @see {@link InitializationData}
         */
        interface Logger {
            /**
             * Prints a message. The message is printed literally, without any decorations such as executable name or
             * timestamp.
             * @param message The message to log.
             * @remarks An implementation of this method must not throw exceptions.
             */
            print(message: string): void;

            /**
             * Logs a trace message.
             * @param category The trace category.
             * @param message The trace message to log.
             * @remarks An implementation of this method must not throw exceptions.
             */
            trace(category: string, message: string): void;

            /**
             * Logs a warning message.
             * @param message The warning message to log.
             * @remarks An implementation of this method must not throw exceptions.
             * @see #error
             */
            warning(message: string): void;

            /**
             * Logs an error message.
             * @param message The error message to log.
             * @remarks An implementation of this method must not throw exceptions.
             * @see #warning
             */
            error(message: string): void;

            /**
             * Returns a clone of the logger with a new prefix.
             * @param prefix The new prefix for the logger.
             * @returns A new logger instance with the specified prefix.
             */
            cloneWithPrefix(prefix: string): Ice.Logger;
        }
    }
}
