// Copyright (c) ZeroC, Inc.

import { Ice as Ice_Context } from "./Context.js";
const { Context, ContextHelper } = Ice_Context;

import { InitializationException } from "./LocalExceptions.js";

//
// The base class for all ImplicitContext implementations
//
export class ImplicitContext {
    constructor() {
        this._context = new Context();
    }

    getContext() {
        return new Context(this._context);
    }

    setContext(context) {
        if (context !== null && context.size > 0) {
            this._context = new Context(context);
        } else {
            this._context.clear();
        }
    }

    containsKey(key) {
        if (key === null) {
            key = "";
        }

        return this._context.has(key);
    }

    get(key) {
        if (key === null) {
            key = "";
        }

        let val = this._context.get(key);
        if (val === null) {
            val = "";
        }

        return val;
    }

    put(key, value) {
        if (key === null) {
            key = "";
        }
        if (value === null) {
            value = "";
        }

        let oldVal = this._context.get(key);
        if (oldVal === null) {
            oldVal = "";
        }

        this._context.set(key, value);

        return oldVal;
    }

    remove(key) {
        if (key === null) {
            key = "";
        }

        let val = this._context.get(key);
        this._context.delete(key);

        if (val === null) {
            val = "";
        }
        return val;
    }

    write(prxContext, os) {
        if (prxContext.size === 0) {
            ContextHelper.write(os, this._context);
        } else {
            let ctx = null;
            if (this._context.size === 0) {
                ctx = prxContext;
            } else {
                ctx = new Context(this._context);
                for (const [key, value] of prxContext) {
                    ctx.set(key, value);
                }
            }
            ContextHelper.write(os, ctx);
        }
    }

    static create(kind) {
        if (kind.length === 0 || kind === "None") {
            return null;
        } else if (kind === "Shared") {
            return new ImplicitContext();
        } else {
            throw new InitializationException(`'${kind}' is not a valid value for Ice.ImplicitContext`);
        }
    }
}
