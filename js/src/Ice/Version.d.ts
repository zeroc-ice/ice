// Copyright (c) ZeroC, Inc.

declare module "ice" {
    namespace Ice {
        /**
         * A version structure for the protocol version.
         */
        class ProtocolVersion {
            constructor(major?: number, minor?: number);
            clone(): ProtocolVersion;
            equals(rhs: any): boolean;
            hashCode(): number;
            major: number;
            minor: number;
            static write(outs: OutputStream, value: ProtocolVersion): void;
            static read(ins: InputStream): ProtocolVersion;
        }

        /**
         * A version structure for the encoding version.
         */
        class EncodingVersion {
            constructor(major?: number, minor?: number);
            clone(): EncodingVersion;
            equals(rhs: any): boolean;
            hashCode(): number;
            major: number;
            minor: number;
            static write(outs: OutputStream, value: EncodingVersion): void;
            static read(ins: InputStream): EncodingVersion;
        }
    }
}
