// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Converts an object identity to a string.
         *
         * @param ident - The object identity to convert.
         * @param toStringMode - Specifies whether and how non-printable ASCII characters are escaped in the result.
         * @returns The string representation of the object identity.
         */
        function identityToString(ident: Identity, toStringMode?: ToStringMode): string;
    }
}
