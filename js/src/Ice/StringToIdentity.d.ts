// Copyright (c) ZeroC, Inc.

declare module "ice" {
    namespace Ice {
        /**
         * Converts a string to an object identity, throwing a {@link ParseException} if the string cannot be
         * converted.
         *
         * @param s - The string to convert.
         * @returns The converted object identity.
         * @throws {@link ParseException} - Thrown if the string cannot be converted to a valid object identity.
         */
        function stringToIdentity(s: string): Identity;
    }
}
