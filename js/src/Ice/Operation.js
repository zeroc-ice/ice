//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;
const _ModuleRegistry = Ice._ModuleRegistry;

require("../Ice/Current");
require("../Ice/Exception");
require("../Ice/FormatType");
require("../Ice/Object");
require("../Ice/ObjectPrx");
require("../Ice/OptionalFormat");
require("../Ice/StreamHelpers");

const builtinHelpers =
[
    Ice.ByteHelper,
    Ice.BoolHelper,
    Ice.ShortHelper,
    Ice.IntHelper,
    Ice.LongHelper,
    Ice.FloatHelper,
    Ice.DoubleHelper,
    Ice.StringHelper,
    Ice.Value,
    Ice.ObjectPrx,
    Ice.Value
];

function parseParam(p)
{
    let type = p[0];
    const t = typeof type;
    if(t === 'number')
    {
        type = builtinHelpers[p[0]];
    }
    else if(t === 'string')
    {
        type = _ModuleRegistry.type(type);
    }

    return {
        type: type,
        isObject: (p[1] === true),
        tag: p[2] // Optional tag, which may not be present - an undefined tag means "not optional".
    };
}

//
// Each operation descriptor is a property. The key is the "on-the-wire"
// name, and the value is an array consisting of the following elements:
//
//  0: native method name in case of a keyword conflict (e.g., "_while"),
//     otherwise an empty string
//  1: mode (undefined == Normal or int)
//  2: sendMode (undefined == Normal or int)
//  3: format (undefined == Default or int)
//  4: return type (undefined if void, or [type, tag])
//  5: in params (undefined if none, or array of [type, tag])
//  6: out params (undefined if none, or array of [type, tag])
//  7: exceptions (undefined if none, or array of types)
//  8: sends classes (true or undefined)
//  9: returns classes (true or undefined)
//
function parseOperation(name, arr)
{
    const r = {};

    r.name = name;
    r.servantMethod = arr[0] ? arr[0] : name;
    r.mode = arr[1] ? Ice.OperationMode.valueOf(arr[1]) : Ice.OperationMode.Normal;
    r.sendMode = arr[2] ? Ice.OperationMode.valueOf(arr[2]) : Ice.OperationMode.Normal;
    r.format = arr[3] ? Ice.FormatType.valueOf(arr[3]) : Ice.FormatType.DefaultFormat;

    let ret;
    if(arr[4])
    {
        ret = parseParam(arr[4]);
        ret.pos = 0;
    }
    r.returns = ret;

    const inParams = [];
    const inParamsOpt = [];
    if(arr[5])
    {
        for(let i = 0; i < arr[5].length; ++i)
        {
            const p = parseParam(arr[5][i]);
            p.pos = i;
            inParams.push(p);
            if(p.tag)
            {
                inParamsOpt.push(p);
            }
        }
    }
    inParamsOpt.sort((p1, p2) => p1.tag - p2.tag); // Sort by tag.
    r.inParams = inParams;
    r.inParamsOpt = inParamsOpt;

    const outParams = [];
    const outParamsOpt = [];
    if(arr[6])
    {
        const offs = ret ? 1 : 0;
        for(let i = 0; i < arr[6].length; ++i)
        {
            const p = parseParam(arr[6][i]);
            p.pos = i + offs;
            outParams.push(p);
            if(p.tag)
            {
                outParamsOpt.push(p);
            }
        }
    }
    if(ret && ret.tag)
    {
        outParamsOpt.push(ret);
    }
    outParamsOpt.sort((p1, p2) => p1.tag - p2.tag); // Sort by tag.
    r.outParams = outParams;
    r.outParamsOpt = outParamsOpt;

    const exceptions = [];
    if(arr[7])
    {
        for(let i = 0; i < arr[7].length; ++i)
        {
            exceptions.push(arr[7][i]);
        }
    }
    r.exceptions = exceptions;

    r.sendsClasses = arr[8] === true;
    r.returnsClasses = arr[9] === true;

    return r;
}

class OpTable
{
    constructor(ops)
    {
        this.raw = ops;
        this.parsed = {};
    }

    find(name)
    {
        //
        // Check if we've already parsed the operation.
        //
        let op = this.parsed[name];
        if(op === undefined && this.raw[name] !== undefined)
        {
            //
            // We haven't parsed it yet, but we found a match for the name, so parse it now.
            //
            op = parseOperation(name, this.raw[name]);
            this.parsed[name] = op;
        }
        return op;
    }
}

function unmarshalParams(is, retvalInfo, allParamInfo, optParamInfo, usesClasses, params, offset)
{
    const readParam = (p, optional) =>
    {
        if(optional)
        {
            if(p.isObject)
            {
                is.readOptionalValue(p.tag,
                                     obj =>
                                     {
                                         params[p.pos + offset] = obj;
                                     },
                                     p.type);
            }
            else
            {
                params[p.pos + offset] = p.type.readOptional(is, p.tag);
            }
        }
        else if(p.isObject)
        {
            is.readValue(obj =>
                         {
                             params[p.pos + offset] = obj;
                         },
                         p.type);
        }
        else
        {
            params[p.pos + offset] = p.type.read(is);
        }
    };

    //
    // First read all required params.
    //
    for(let i = 0; i < allParamInfo.length; ++i)
    {
        if(!allParamInfo[i].tag)
        {
            readParam(allParamInfo[i], false);
        }
    }

    //
    // Then read a required return value (if any).
    //
    if(retvalInfo)
    {
        readParam(retvalInfo, false);
    }

    //
    // Then read all optional params.
    //
    for(let i = 0; i < optParamInfo.length; ++i)
    {
        readParam(optParamInfo[i], true);
    }

    if(usesClasses)
    {
        is.readPendingValues();
    }
}

function marshalParams(os, params, retvalInfo, paramInfo, optParamInfo, usesClasses)
{
    //
    // Write the required params.
    //
    for(let i = 0; i < paramInfo.length; ++i)
    {
        const p = paramInfo[i];
        if(!p.tag)
        {
            p.type.write(os, params[p.pos]);
        }
    }

    //
    // retvalInfo should only be provided if there is a non-void required return value.
    //
    if(retvalInfo)
    {
        retvalInfo.type.write(os, params[retvalInfo.pos]);
    }

    //
    // Write the optional params.
    //
    for(let i = 0; i < optParamInfo.length; ++i)
    {
        const p = optParamInfo[i];
        p.type.writeOptional(os, p.tag, params[p.pos]);
    }

    if(usesClasses)
    {
        os.writePendingValues();
    }
}

function dispatchImpl(servant, op, incomingAsync, current)
{
    //
    // Check to make sure the servant implements the operation.
    //
    const method = servant[op.servantMethod];
    if(method === undefined || typeof method !== "function")
    {
        throw new Ice.UnknownException("servant for identity " + current.adapter.getCommunicator().identityToString(current.id) +
                                       " does not define operation `" + op.servantMethod + "'");
    }

    //
    // Unmarshal the in params (if any).
    //
    const params = [];
    if(op.inParams.length === 0)
    {
        incomingAsync.readEmptyParams();
    }
    else
    {
        const is = incomingAsync.startReadParams();
        unmarshalParams(is, undefined, op.inParams, op.inParamsOpt, op.sendsClasses, params, 0);
        incomingAsync.endReadParams();
    }

    params.push(current);

    incomingAsync.setFormat(op.format);

    const marshalFn = function(params)
    {
        const numExpectedResults = op.outParams.length + (op.returns ? 1 : 0);
        if(numExpectedResults > 1 && !(params instanceof Array))
        {
            throw new Ice.MarshalException("operation `" + op.servantMethod + "' should return an array");
        }
        else if(numExpectedResults === 1)
        {
            params = [params]; // Wrap a single out parameter in an array.
        }

        if(op.returns === undefined && op.outParams.length === 0)
        {
            if(params && params.length > 0)
            {
                throw new Ice.MarshalException("operation `" + op.servantMethod + "' shouldn't return any value");
            }
            else
            {
                incomingAsync.writeEmptyParams();
            }
        }
        else
        {
            let retvalInfo;
            if(op.returns && !op.returns.tag)
            {
                retvalInfo = op.returns;
            }

            const os = incomingAsync.startWriteParams();
            marshalParams(os, params, retvalInfo, op.outParams, op.outParamsOpt, op.returnsClasses);
            incomingAsync.endWriteParams();
        }
    };

    const results = method.apply(servant, params);
    if(results instanceof Promise)
    {
        return results.then(marshalFn);
    }
    else
    {
        marshalFn(results);
        return null;
    }
}

function getServantMethodFromInterfaces(interfaces, methodName, all)
{
    let method;
    for(let i = 0; method === undefined && i < interfaces.length; ++i)
    {
        const intf = interfaces[i];
        method = intf[methodName];
        if(method === undefined)
        {
            if(all.indexOf(intf) === -1)
            {
                all.push(intf);
            }
            if(intf._iceImplements)
            {
                method = getServantMethodFromInterfaces(intf._iceImplements, methodName, all);
            }
        }
    }
    return method;
}

const dispatchPrefix = "_iceD_";

function getServantMethod(servantType, name)
{
    //
    // The dispatch method is named _iceD_<Slice name> and is stored in the type (not the prototype).
    //
    const methodName = dispatchPrefix + name;

    //
    // First check the servant type.
    //
    let method = servantType[methodName];

    let allInterfaces;

    if(method === undefined)
    {
        allInterfaces = [];

        //
        // Now check the prototypes of the implemented interfaces.
        //
        let curr = servantType;
        while(curr && method === undefined)
        {
            if(curr._iceImplements)
            {
                method = getServantMethodFromInterfaces(curr._iceImplements, methodName, allInterfaces);
            }
            curr = Object.getPrototypeOf(curr);
        }

        if(method !== undefined)
        {
            //
            // Add the method to the servant's type.
            //
            servantType[methodName] = method;
        }
    }

    if(method === undefined)
    {
        //
        // Next check the op table for the servant's type.
        //
        let op;
        if(servantType._iceOps)
        {
            op = servantType._iceOps.find(name);
        }

        let source;
        if(op === undefined)
        {
            //
            // Now check the op tables of the base types.
            //
            let parent = Object.getPrototypeOf(servantType);
            while(op === undefined && parent)
            {
                if(parent._iceOps)
                {
                    if((op = parent._iceOps.find(name)) !== undefined)
                    {
                        source = parent;
                    }
                }
                parent = Object.getPrototypeOf(parent);
            }

            //
            // Now check the op tables of all base interfaces.
            //
            for(let i = 0; op === undefined && i < allInterfaces.length; ++i)
            {
                const intf = allInterfaces[i];
                if(intf._iceOps)
                {
                    if((op = intf._iceOps.find(name)) !== undefined)
                    {
                        source = intf;
                    }
                }
            }
        }

        if(op !== undefined)
        {
            method = function(servant, incomingAsync, current)
            {
                return dispatchImpl(servant, op, incomingAsync, current);
            };

            //
            // Add the method to the servant type.
            //
            servantType[methodName] = method;

            //
            // Also add the method to the type in which the operation was found.
            //
            if(source)
            {
                source[methodName] = method;
            }
        }
    }

    return method;
}

function addProxyOperation(proxyType, name, data)
{
    const method = data[0] ? data[0] : name;

    let op = null;

    proxyType.prototype[method] = function(...args)
    {
        //
        // Parse the operation data on the first invocation of a proxy method.
        //
        if(op === null)
        {
            op = parseOperation(name, data);
        }

        const ctx = args[op.inParams.length]; // The request context is the last argument (if present).

        let marshalFn = null;
        if(op.inParams.length > 0)
        {
            marshalFn = function(os, params)
            {
                //
                // Validate the parameters.
                //
                for(let i = 0; i < op.inParams.length; ++i)
                {
                    const p = op.inParams[i];
                    const v = params[p.pos];
                    if(!p.tag || v !== undefined)
                    {
                        if(typeof p.type.validate === "function")
                        {
                            if(!p.type.validate(v))
                            {
                                throw new Ice.MarshalException("invalid value for argument " + (i + 1) +
                                                               " in operation `" + op.servantMethod + "'");
                            }
                        }
                    }
                }

                marshalParams(os, params, undefined, op.inParams, op.inParamsOpt, op.sendsClasses);
            };
        }

        let unmarshalFn = null;
        if(op.returns || op.outParams.length > 0)
        {
            unmarshalFn = function(asyncResult)
            {
                //
                // The results array holds the out parameters in the following format:
                //
                // [retval, out1, out2, ..., asyncResult]
                //
                const results = [];

                const is = asyncResult.startReadParams();
                let retvalInfo;
                if(op.returns && !op.returns.tag)
                {
                    retvalInfo = op.returns;
                }
                unmarshalParams(is, retvalInfo, op.outParams, op.outParamsOpt, op.returnsClasses, results, 0);
                asyncResult.endReadParams();
                return results.length == 1 ? results[0] : results;
            };
        }
        return Ice.ObjectPrx._invoke(this, op.name, op.sendMode, op.format, ctx, marshalFn, unmarshalFn,
                                     op.exceptions, Array.prototype.slice.call(args));
    };
}

const Slice = Ice.Slice;
Slice.defineOperations = function(classType, proxyType, ids, pos, ops)
{
    if(ops)
    {
        classType._iceOps = new OpTable(ops);
    }

    classType.prototype._iceDispatch = function(incomingAsync, current)
    {
        //
        // Retrieve the dispatch method for this operation.
        //
        const method = getServantMethod(classType, current.operation);

        if(method === undefined || typeof method !== 'function')
        {
            throw new Ice.OperationNotExistException(current.id, current.facet, current.operation);
        }

        return method.call(method, this, incomingAsync, current);
    };

    classType.prototype._iceMostDerivedType = function()
    {
        return classType;
    };

    Object.defineProperty(classType, "_iceIds", {
        get: () => ids
    });

    Object.defineProperty(classType, "_iceId", {
        get: () => ids[pos]
    });

    classType.ice_staticId = function()
    {
        return classType._iceId;
    };

    if(proxyType !== undefined)
    {
        if(ops)
        {
            for(const name in ops)
            {
                addProxyOperation(proxyType, name, ops[name]);
            }
        }

        //
        // Copy proxy methods from super-interfaces.
        //
        if(proxyType._implements)
        {
            for(const intf in proxyType._implements)
            {
                const proto = proxyType._implements[intf].prototype;
                for(const f in proto)
                {
                    if(typeof proto[f] == "function" && proxyType.prototype[f] === undefined)
                    {
                        proxyType.prototype[f] = proto[f];
                    }
                }
            }
        }

        Object.defineProperty(proxyType, "_id", {
            get: () => ids[pos]
        });
    }
};

//
// Define the "built-in" operations for all Ice objects.
//
Slice.defineOperations(Ice.Object, Ice.ObjectPrx, ["::Ice::Object"], 0,
{
    ice_ping: [undefined, 1, 1, undefined, undefined, undefined, undefined, undefined],
    ice_isA: [undefined, 1, 1, undefined, [1], [[7]], undefined, undefined],
    ice_id: [undefined, 1, 1, undefined, [7], undefined, undefined, undefined],
    ice_ids: [undefined, 1, 1, undefined, ["Ice.StringSeqHelper"], undefined, undefined, undefined]
});

module.exports.Ice = Ice;
