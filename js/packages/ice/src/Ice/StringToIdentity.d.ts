// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Converts a string to an object identity.
         *
         * @param s - The string to convert.
         * @returns The converted object identity.
         * @throws {@link ParseException} - Thrown if the string cannot be parsed.
         */
        function stringToIdentity(s: string): Identity;
    }
}
