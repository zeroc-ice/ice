// Copyright (c) ZeroC, Inc.

declare module "ice" {
    namespace Ice {
        class ByteHelper {
            static validate(v: number): boolean;
        }

        class ShortHelper {
            static validate(v: number): boolean;
        }

        class IntHelper {
            static validate(v: number): boolean;
        }

        class FloatHelper {
            static validate(v: number): boolean;
        }

        class DoubleHelper {
            static validate(v: number): boolean;
        }

        class LongHelper {
            static validate(v: Long): boolean;
        }
    }
}
