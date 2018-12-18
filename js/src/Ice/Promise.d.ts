// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

export class P<T> extends Promise<T>
{
    constructor(executor?:(resolve:(value:T|PromiseLike<T>)=>void, reject:(reason:any)=>void) => void);
    finally(callback:() => void | PromiseLike<void>): T | PromiseLike<T>;
    delay(ms:number): P<T>;
    resolve<T>(value?:T|PromiseLike<T>):void;
    reject<T>(reason:any):void;
    static delay(ms:number):P<void>;
    static delay<T>(ms:number, value:T):P<T>;
    static try<T>(cb:()=>T|PromiseLike<T>):P<T>;
}

declare module "ice"
{
    namespace Ice
    {
        export {P as Promise};
    }
}
