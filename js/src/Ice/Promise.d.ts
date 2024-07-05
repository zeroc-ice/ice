//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

declare module "ice" {
    namespace Ice {
        class Promise<T> extends globalThis.Promise<T> {
            constructor(
                executor?: (resolve: (value: T | PromiseLike<T>) => void, reject: (reason: any) => void) => void,
            );
            delay(ms: number): Promise<T>;
            resolve<T>(value?: T | PromiseLike<T>): void;
            reject<T>(reason: any): void;
            static delay(ms: number): Promise<void>;
            static delay<T>(ms: number, value: T): Promise<T>;
            static try<T>(cb: () => T | PromiseLike<T>): Promise<T>;
        }
    }
}
