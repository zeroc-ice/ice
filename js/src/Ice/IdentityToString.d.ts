//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice" {
    namespace Ice {
        /**
         * Converts an object identity to a string.
         *
         * @param ident The object identity to convert.
         * @param toStringMode Specifies if and how non-printable ASCII characters are escaped in the result.
         * @returns The string representation of the object identity.
         */
        function identityToString(ident: Identity, toStringMode?: ToStringMode): string;
    }
}
