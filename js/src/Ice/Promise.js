// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/Timer").Ice;
const Timer = Ice.Timer;

class P extends Promise
{
    constructor(cb)
    {
        let res;
        let rej;
        super((resolve, reject) =>
            {
                res = resolve;
                rej = reject;

                if(cb !== undefined)
                {
                    cb(resolve, reject);
                }
            });

        this.resolve = res;
        this.reject = rej;
    }

    finally(cb)
    {
        return this.then(
            value => P.resolve(cb()).then(() => value),
            reason => P.resolve(cb()).then(() => { throw reason; }));
    }

    delay(ms)
    {
        return this.then(
            value => new P((resolve, reject) => Timer.setTimeout(() => resolve(value), ms)),
            reason => new P((resolve, reject) => Timer.setTimeout(() => reject(reason), ms)));
    }

    static get [Symbol.species]()
    {
        return P;
    }

    static delay(ms, value)
    {
        return new P(resolve => Timer.setTimeout(() => resolve(value), ms));
    }

    static try(cb)
    {
        return P.resolve().then(cb);
    }
}

Ice.Promise = P;
module.exports.Ice = Ice;
