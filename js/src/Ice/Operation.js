// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


var Ice = require("../Ice/ModuleRegistry").Ice;
var __M = Ice.__M;
__M.require(module,
    [
        "../Ice/Class",
        "../Ice/Current",
        "../Ice/DispatchStatus",
        "../Ice/Exception",
        "../Ice/FormatType",
        "../Ice/Object",
        "../Ice/ObjectPrx",
        "../Ice/OptionalFormat",
        "../Ice/StreamHelpers"
    ]);

var Class = Ice.Class;

var builtinHelpers =
[
    Ice.ByteHelper,
    Ice.BoolHelper,
    Ice.ShortHelper,
    Ice.IntHelper,
    Ice.LongHelper,
    Ice.FloatHelper,
    Ice.DoubleHelper,
    Ice.StringHelper,
    Ice.Object,
    Ice.ObjectPrx
];

function parseParam(p)
{
    var type = p[0];
    var t = typeof(type);
    if(t === 'number')
    {
        type = builtinHelpers[p[0]];
    }
    else if(t === 'string')
    {
        /*jshint -W061 */
        type = __M.type(type);
        /*jshint +W061 */
    }

    return {
        "type": type,
        "isObject": (p[1] === true),
        "tag": p[2] // Optional tag, which may not be present - an undefined tag means "not optional".
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
//  3: amd (undefined or 1)
//  4: format (undefined == Default or int)
//  5: return type (undefined if void, or [type, tag])
//  6: in params (undefined if none, or array of [type, tag])
//  7: out params (undefined if none, or array of [type, tag])
//  8: exceptions (undefined if none, or array of types)
//  9: sends classes (true or undefined)
// 10: returns classes (true or undefined)
//
function parseOperation(name, arr)
{
    var r = {};
    var i;
    var p;

    r.name = name;
    r.mode = arr[1] ? Ice.OperationMode.valueOf(arr[1]) : Ice.OperationMode.Normal;
    r.sendMode = arr[2] ? Ice.OperationMode.valueOf(arr[2]) : Ice.OperationMode.Normal;
    r.amd = arr[3] ? true : false;
    r.format = arr[4] ? Ice.FormatType.valueOf(arr[4]) : Ice.FormatType.DefaultFormat;

    if(r.amd)
    {
        r.servantMethod = name + "_async";
    }
    else
    {
        r.servantMethod = arr[0] ? arr[0] : name;
    }

    var ret;
    if(arr[5])
    {
        ret = parseParam(arr[5]);
        ret.pos = 0;
    }
    r.returns = ret;

    var inParams = [];
    var inParamsOpt = [];
    if(arr[6])
    {
        for(i = 0; i < arr[6].length; ++i)
        {
            p = parseParam(arr[6][i]);
            p.pos = i;
            inParams.push(p);
            if(p.tag)
            {
                inParamsOpt.push(p);
            }
        }
    }
    inParamsOpt.sort(function(p1, p2) { return p1.tag - p2.tag; }); // Sort by tag.
    r.inParams = inParams;
    r.inParamsOpt = inParamsOpt;

    var outParams = [];
    var outParamsOpt = [];
    if(arr[7])
    {
        var offs = ret ? 1 : 0;
        for(i = 0; i < arr[7].length; ++i)
        {
            p = parseParam(arr[7][i]);
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
    outParamsOpt.sort(function(p1, p2) { return p1.tag - p2.tag; }); // Sort by tag.
    r.outParams = outParams;
    r.outParamsOpt = outParamsOpt;

    var exceptions = [];
    if(arr[8])
    {
        for(i = 0; i < arr[8].length; ++i)
        {
            exceptions.push(arr[8][i]);
        }
    }
    r.exceptions = exceptions;

    r.sendsClasses = arr[9] === true;
    r.returnsClasses = arr[10] === true;

    return r;
}

var OpTable = Class({
    __init__: function(ops)
    {
        this.raw = ops;
        this.parsed = {};
    },
    find: function(name)
    {
        //
        // Check if we've already parsed the operation.
        //
        var op = this.parsed[name];
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
});

function unmarshalParams(is, retvalInfo, allParamInfo, optParamInfo, usesClasses, params, offset)
{
    var i, p, v;
    //
    // First read all required params.
    //
    for(i = 0; i < allParamInfo.length; ++i)
    {
        p = allParamInfo[i];
        if(!p.tag)
        {
            v = p.type.read(is);
            params[p.pos + offset] = v;
        }
    }

    //
    // Then read a required return value (if any).
    //
    if(retvalInfo)
    {
        v = retvalInfo.type.read(is);
        params[retvalInfo.pos + offset] = v;
    }

    //
    // Then read all optional params.
    //
    for(i = 0; i < optParamInfo.length; ++i)
    {
        p = optParamInfo[i];
        v = p.type.readOptional(is, p.tag);
        params[p.pos + offset] = v;
    }

    if(usesClasses)
    {
        is.readPendingValues();
    }
}

function marshalParams(os, params, retvalInfo, paramInfo, optParamInfo, usesClasses)
{
    var i, p;

    //
    // Write the required params.
    //
    for(i = 0; i < paramInfo.length; ++i)
    {
        p = paramInfo[i];
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
    for(i = 0; i < optParamInfo.length; ++i)
    {
        p = optParamInfo[i];
        p.type.writeOptional(os, p.tag, params[p.pos]);
    }

    if(usesClasses)
    {
        os.writePendingValues();
    }
}

var Upcall = Class({
    __init__: function(incomingAsync, op)
    {
        this.incomingAsync = incomingAsync;
        this.op = op;
    },
    ice_response: function()
    {
        var args = arguments;

        if(this.incomingAsync.__validateResponse(true))
        {
            try
            {
                this.__sendResponse(args);
                this.incomingAsync.__response();
            }
            catch(ex)
            {
                this.incomingAsync.__exception(ex);
            }
        }
    },
    ice_exception: function(ex)
    {
        if(this.__checkException(ex))
        {
            if(this.incomingAsync.__validateResponse(false))
            {
                this.__sendException(ex);
                this.incomingAsync.__response();
            }
        }
        else
        {
            this.incomingAsync.ice_exception(ex);
        }
    },
    __sendResponse: function(results)
    {
        if(this.op.returns === undefined && this.op.outParams.length === 0)
        {
            if(results && results.length > 0)
            {
                //
                // No results expected.
                //
                throw new Error("ice_response called with invalid arguments");
            }
            else
            {
                this.incomingAsync.__writeEmptyParams();
            }
        }
        else
        {
            var __os = this.incomingAsync.__startWriteParams(this.op.format);
            var retvalInfo;
            if(this.op.returns && !this.op.returns.tag)
            {
                retvalInfo = this.op.returns;
            }
            marshalParams(__os, results, retvalInfo, this.op.outParams, this.op.outParamsOpt,
                          this.op.returnsClasses);
            this.incomingAsync.__endWriteParams(true);
        }
    },
    __checkException: function(ex)
    {
        //
        // Make sure the given exception is an instance of one of the declared user exceptions
        // for this operation.
        //
        for(var i = 0; i < this.op.exceptions.length; ++i)
        {
            if(ex instanceof this.op.exceptions[i])
            {
                //
                // User exception is valid.
                //
                return true;
            }
        }

        return false;
    },
    __sendException: function(ex)
    {
        //
        // User exception is valid, now marshal it.
        //
        this.incomingAsync.__writeUserException(ex, this.op.format);
    }
});

var __dispatchImpl = function(servant, op, incomingAsync, current)
{
    //
    // Check to make sure the servant implements the operation.
    //
    var method = servant[op.servantMethod];
    if(method === undefined || typeof(method) !== "function")
    {
        var comm = current.adapter.getCommunicator();
        var msg = "servant for identity " + Ice.identityToString(current.id) +
            " does not define operation `" + op.servantMethod + "'";
        throw new Ice.UnknownException(msg);
    }

    var up = new Upcall(incomingAsync, op);

    try
    {
        //
        // Unmarshal the in params (if any).
        //
        var params = op.amd ? [null] : [];
        if(op.inParams.length === 0)
        {
            incomingAsync.readEmptyParams();
        }
        else
        {
            var __is = incomingAsync.startReadParams();
            var offset = op.amd ? 1 : 0;
            unmarshalParams(__is, undefined, op.inParams, op.inParamsOpt, op.sendsClasses, params, offset);
            incomingAsync.endReadParams();

            //
            // When unmarshaling objects, the ObjectHelper returns a wrapper object
            // and eventually stores the unmarshaled object into its "value" member.
            // Here we scan the parameter array and replace the wrappers with the
            // actual object references.
            //
            if(op.inParams.length > 0 && (op.sendsClasses || op.inParamsOpt.length > 0))
            {
                for(var i = 0; i < op.inParams.length; ++i)
                {
                    var p = op.inParams[i];
                    //
                    // Optional parameters may be undefined.
                    //
                    if(p.isObject && params[p.pos + offset] !== undefined)
                    {
                        params[p.pos + offset] = params[p.pos + offset].value;
                    }
                }
            }
        }

        params.push(current);

        if(op.amd)
        {
            params[0] = up; // The AMD callback object.
            try
            {
                method.apply(servant, params);
            }
            catch(ex)
            {
                up.ice_exception(ex);
            }
            return Ice.DispatchStatus.DispatchAsync;
        }
        else
        {
            //
            // Determine how many out parameters to expect.
            //
            var numExpectedResults = op.outParams.length;
            if(op.returns)
            {
                ++numExpectedResults;
            }

            var results = method.apply(servant, params);

            //
            // Complain if we expect more than out parameter and the servant doesn't return an array.
            //
            if(numExpectedResults > 1 && !(results instanceof Array))
            {
                throw new Ice.MarshalException("operation `" + op.servantMethod +
                                               "' should return an array of length " + numExpectedResults);
            }
            else if(numExpectedResults === 1)
            {
                //
                // Wrap a single out parameter in an array.
                //
                results = [results];
            }

            up.__sendResponse(results);
            return Ice.DispatchStatus.DispatchOK;
        }
    }
    catch(ex)
    {
        if(up.__checkException(ex))
        {
            up.__sendException(ex);
            return Ice.DispatchStatus.DispatchUserException;
        }
        else
        {
            throw ex;
        }
    }
};

function getServantMethodFromInterfaces(interfaces, methodName, all)
{
    var method;
    for(var i = 0; method === undefined && i < interfaces.length; ++i)
    {
        var intf = interfaces[i];
        method = intf[methodName];
        if(method === undefined)
        {
            if(all.indexOf(intf) === -1)
            {
                all.push(intf);
            }
            if(intf.__implements)
            {
                method = getServantMethodFromInterfaces(intf.__implements, methodName, all);
            }
        }
    }
    return method;
}

var dispatchPrefix = "__op_";

function getServantMethod(servantType, name)
{
    //
    // The dispatch method is named __op_<Slice name> and is stored in the type (not the prototype).
    //
    var methodName = dispatchPrefix + name;

    //
    // First check the servant type.
    //
    var method = servantType[methodName];

    var allInterfaces;

    if(method === undefined)
    {
        allInterfaces = [];

        //
        // Now check the prototypes of the implemented interfaces.
        //
        var curr = servantType;
        while(curr && method === undefined)
        {
            if(curr.__implements)
            {
                method = getServantMethodFromInterfaces(curr.__implements, methodName, allInterfaces);
            }
            curr = curr.__parent;
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
        var op;
        if(servantType.__ops)
        {
            op = servantType.__ops.find(name);
        }

        var source;
        if(op === undefined)
        {
            //
            // Now check the op tables of the base types.
            //
            var parent = servantType.__parent;
            while(op === undefined && parent)
            {
                if(parent.__ops)
                {
                    if((op = parent.__ops.find(name)) !== undefined)
                    {
                        source = parent;
                    }
                }
                parent = parent.__parent;
            }

            //
            // Now check the op tables of all base interfaces.
            //
            for(var i = 0; op === undefined && i < allInterfaces.length; ++i)
            {
                var intf = allInterfaces[i];
                if(intf.__ops)
                {
                    if((op = intf.__ops.find(name)) !== undefined)
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
                return __dispatchImpl(servant, op, incomingAsync, current);
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
    var method = data[0] ? data[0] : name;

    var op = null;

    proxyType.prototype[method] = function()
    {
        var args = arguments;

        //
        // Parse the operation data on the first invocation of a proxy method.
        //
        if(op === null)
        {
            op = parseOperation(name, data);
        }

        var ctx = args[op.inParams.length]; // The request context is the last argument (if present).

        var marshalFn = null;
        if(op.inParams.length > 0)
        {
            marshalFn = function(os, params)
            {
                var i, p, v;

                //
                // Validate the parameters.
                //
                for(i = 0; i < op.inParams.length; ++i)
                {
                    p = op.inParams[i];
                    v = params[p.pos];
                    if(!p.tag || v !== undefined)
                    {
                        if(typeof p.type.validate === "function")
                        {
                            if(!p.type.validate(v))
                            {
                                throw new Ice.MarshalException("invalid value for argument " + (i + 1)  +
                                                               " in operation `" + op.servantMethod + "'");
                            }
                        }
                    }
                }

                marshalParams(os, params, undefined, op.inParams, op.inParamsOpt, op.sendsClasses);
            };
        }

        var unmarshalFn = null;
        if(op.returns || op.outParams.length > 0)
        {
            unmarshalFn = function(asyncResult)
            {
                //
                // The results array holds the out parameters in the following format:
                //
                // [retval, out1, out2, ..., asyncResult]
                //
                var results = [];

                var is = asyncResult.__startReadParams();

                var retvalInfo;
                if(op.returns && !op.returns.tag)
                {
                    retvalInfo = op.returns;
                }
                unmarshalParams(is, retvalInfo, op.outParams, op.outParamsOpt, op.returnsClasses, results, 0);

                asyncResult.__endReadParams();

                //
                // When unmarshaling objects, the ObjectHelper returns a wrapper object
                // and eventually stores the unmarshaled object into its "value" member.
                // Here we scan the results array and replace the wrappers with the
                // actual object references.
                //
                if(op.returnsClasses || op.outParamsOpt.length > 0)
                {
                    var offset = 0; // Skip asyncResult in results.
                    if(op.returns && op.returns.isObject && results[op.returns.pos + offset] !== undefined)
                    {
                        results[op.returns.pos + offset] = results[op.returns.pos + offset].value;
                    }
                    for(var i = 0; i < op.outParams.length; ++i)
                    {
                        var p = op.outParams[i];
                        //
                        // Optional parameters may be undefined.
                        //
                        if(p.isObject && results[p.pos + offset] !== undefined)
                        {
                            results[p.pos + offset] = results[p.pos + offset].value;
                        }
                    }
                }
                results.push(asyncResult);
                return results;
            };
        }

        return Ice.ObjectPrx.__invoke(this, op.name, op.sendMode, op.format, ctx, marshalFn, unmarshalFn,
                                        op.exceptions, Array.prototype.slice.call(args));
    };
}

var Slice = Ice.Slice;
Slice.defineOperations = function(classType, proxyType, ops)
{
    if(ops)
    {
        classType.__ops = new OpTable(ops);
    }

    classType.prototype.__dispatch = function(incomingAsync, current)
    {
        //
        // Retrieve the dispatch method for this operation.
        //
        var method = getServantMethod(classType, current.operation);

        if(method === undefined || typeof(method) !== 'function')
        {
            throw new Ice.OperationNotExistException(current.id, current.facet, current.operation);
        }

        return method.call(method, this, incomingAsync, current);
    };

    if(ops)
    {
        for(var name in ops)
        {
            addProxyOperation(proxyType, name, ops[name]);
        }
    }

    //
    // Copy proxy methods from super-interfaces.
    //
    if(proxyType.__implements)
    {
        for(var intf in proxyType.__implements)
        {
            var proto = proxyType.__implements[intf].prototype;
            for(var f in proto)
            {
                if(typeof proto[f] == "function" && proxyType.prototype[f] === undefined)
                {
                    proxyType.prototype[f] = proto[f];
                }
            }
        }
    }
};

//
// Define the "built-in" operations for all Ice objects.
//
Slice.defineOperations(Ice.Object, Ice.ObjectPrx,
{
    "ice_ping": [, 1, 1, , , , , , ],
    "ice_isA": [, 1, 1, , , [1], [[7]], , ],
    "ice_id": [, 1, 1, , , [7], , , ],
    "ice_ids": [, 1, 1, , , ["Ice.StringSeqHelper"], , , ]
});

module.exports.Ice = Ice;
