//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice" {
    namespace Ice {
        /**
         * The identity of an Ice object. In a proxy, an empty {@link Identity#name} denotes a nil proxy. An identity with
         * an empty {@link Identity#name} and a non-empty {@link Identity#category} is illegal. You cannot add a servant
         * with an empty name to the Active Servant Map.
         * @see ServantLocator
         * @see ObjectAdapter#addServantLocator
         */
        class Identity {
            /**
             * Constructs a new Identity instance.
             *
             * @param name The identity name
             * @param category The identity category
             */
            constructor(name?: string, category?: string);

            /**
             * Clones the identity.
             *
             * @returns A shallow clone of the identity.
             */
            clone(): Identity;

            /**
             * Determines whether the specified object is equal to this object.
             *
             * @param other The object to compare with.
             * @returns `true` if the specified object is equal to the this object, `false` otherwise.
             */
            equals(other: any): boolean;

            /**
             * Returns the hash code of the object.
             *
             * @returns the hash code of the object.
             */
            hashCode(): number;

            /**
             * The identity name.
             */
            name: string;

            /**
             * The identity category.
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
             * @param outs The OutputStream to write to.
             * @param value The value to write.
             */
            static write(outs: OutputStream, value: IdentitySeq): void;

            /**
             * Reads {@link IdentitySeq} from the given InputStream.
             *
             * @param ins The InputStream to read from.
             * @returns The read {@link IdentitySeq} value.
             */
            static read(ins: InputStream): IdentitySeq;
        }
    }
}
