//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

export class P<T> extends Promise<T>
{
    constructor(executor?:(resolve:(value:T|PromiseLike<T>)=>void, reject:(reason:any)=>void) => void);
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
