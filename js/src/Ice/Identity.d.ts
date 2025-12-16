// Copyright (c) ZeroC, Inc.

declare module "@zeroc/ice" {
    namespace Ice {
        /**
         * Represents the identity of an Ice object. It is comparable to the path of a URI. Its string representation
         * is `name` when the category is empty, and `category/name` when the category is not empty.
         */
        class Identity {
            /**
             * Constructs a new `Identity` object.
             *
             * @param name - The identity name.
             * @param category - The identity category.
             */
            constructor(name?: string, category?: string);

            /**
             * Clones this identity.
             *
             * @returns A shallow copy of the identity.
             */
            clone(): Identity;

            /**
             * Determines whether the specified object is equal to this object.
             *
             * @param other - The object to compare with.
             * @returns `true` if the specified object is equal to the this object, `false` otherwise.
             */
            equals(other: Identity): boolean;

            /**
             * Returns the hash code of the object.
             *
             * @returns the hash code of the object.
             */
            hashCode(): number;

            /**
             * The name of the Ice object. An empty name is not valid.
             */
            name: string;

            /**
             * The category of the object.
             */
            category: string;

            /**
             * Writes the {@link Identity} value to the given OutputStream.
             *
             * @param outs The OutputStream to write to.
             * @param value The value to write.
             */
            static write(outs: OutputStream, value: Identity): void;

            /**
             * Reads {@link Identity} from the given InputStream.
             *
             * @param ins The InputStream to read from.
             * @returns The read {@link Identity} value.
             */
            static read(ins: InputStream): Identity;
        }

        /**
         * A sequence of identities.
         */
        type IdentitySeq = Identity[];

        /**
         * Helper class for encoding and decoding {@link IdentitySeq}.
         */
        class IdentitySeqHelper {
            /**
             * Writes the {@link IdentitySeq} value to the given OutputStream.
             *
             * @param outs - The OutputStream to write to.
             * @param value - The value to write.
             */
            static write(outs: OutputStream, value: IdentitySeq): void;

            /**
             * Reads {@link IdentitySeq} from the given InputStream.
             *
             * @param ins - The InputStream to read from.
             * @returns The read {@link IdentitySeq} value.
             */
            static read(ins: InputStream): IdentitySeq;
        }
    }
}
