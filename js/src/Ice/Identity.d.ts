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
            constructor(name?: string, category?: string);
            clone(): Identity;
            equals(rhs: any): boolean;
            hashCode(): number;
            name: string;
            category: string;
            static write(outs: OutputStream, value: Identity): void;
            static read(ins: InputStream): Identity;
        }

        /**
         * A sequence of identities.
         */
        type IdentitySeq = Identity[];

        class IdentitySeqHelper {
            static write(outs: OutputStream, value: IdentitySeq): void;
            static read(ins: InputStream): IdentitySeq;
        }
    }
}
