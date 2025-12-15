// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Converts an Identity into a string using the specified mode.
         *
         * @param ident - The identity.
         * @param toStringMode - Specifies how to handle non-ASCII characters and non-printable ASCII characters.
         * @returns The stringified identity.
         */
        function identityToString(ident: Identity, toStringMode?: ToStringMode): string;
    }
}
