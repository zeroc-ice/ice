//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

export namespace Ice
{
    /**
     * The Ice message logger. Applications can provide their own logger
     * by implementing this interface and installing it in a communicator.
     */
    interface Logger
    {
        /**
         * Print a message. The message is printed literally, without
         * any decorations such as executable name or time stamp.
         * @param message The message to log.
         */
        print(message:string):void;
        /**
         * Log a trace message.
         * @param category The trace category.
         * @param message The trace message to log.
         */
        trace(category:string, message:string):void;
        /**
         * Log a warning message.
         * @param message The warning message to log.
         * @see #error
         */
        warning(message:string):void;
        /**
         * Log an error message.
         * @param message The error message to log.
         * @see #warning
         */
        error(message:string):void;
        /**
         * Returns this logger's prefix.
         * @return The prefix.
         */
        getPrefix():string;
        /**
         * Returns a clone of the logger with a new prefix.
         * @param prefix The new prefix for the logger.
         * @return A logger instance.
         */
        cloneWithPrefix(prefix:string):Ice.Logger;
    }
}
