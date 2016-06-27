// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class",
        "../Ice/Stream",
        "../Ice/BuiltinSequences",
        "../Ice/Connection",
        "../Ice/Current",
        "../Ice/Debug",
        "../Ice/DispatchStatus",
        "../Ice/Exception",
        "../Ice/Identity",
        "../Ice/LocalException",
        "../Ice/Protocol",
        "../Ice/StringUtil"
    ]);

var OutputStream = Ice.OutputStream;
var Current = Ice.Current;
var Debug = Ice.Debug;
var FormatType = Ice.FormatType;
var Context = Ice.Context;
var Identity = Ice.Identity;
var Protocol = Ice.Protocol;
var StringUtil = Ice.StringUtil;

var IncomingAsync = Ice.Class({
    __init__: function(instance, connection, adapter, response, compress, requestId)
    {
        this._instance = instance;
        this._response = response;
        this._compress = compress;
        if(this._response)
        {
            this._os = new OutputStream(instance, Protocol.currentProtocolEncoding);
        }
        this._connection = connection;

        this._current = new Current();
        this._current.id = new Identity();
        this._current.adapter = adapter;
        this._current.con = this._connection;
        this._current.requestId = requestId;

        this._servant = null;
        this._locator = null;
        this._cookie = { value: null };

        //
        // Prepare the response if necessary.
        //
        if(response)
        {
            this._os.writeBlob(Protocol.replyHdr);

            //
            // Add the request ID.
            //
            this._os.writeInt(requestId);
        }

        this._is = null;

        this._cb = null;
        this._active = true;
    },
    __startWriteParams: function(format)
    {
        if(!this._response)
        {
            throw new Ice.MarshalException("can't marshal out parameters for oneway dispatch");
        }

        Debug.assert(this._os.size == Protocol.headerSize + 4); // Reply status position.
        Debug.assert(this._current.encoding !== null); // Encoding for reply is known.
        this._os.writeByte(0);
        this._os.startEncapsulation(this._current.encoding, format);
        return this._os;
    },
    __endWriteParams: function(ok)
    {
        if(this._response)
        {
            var save = this._os.pos;
            this._os.pos = Protocol.headerSize + 4; // Reply status position.
            this._os.writeByte(ok ? Protocol.replyOK : Protocol.replyUserException);
            this._os.pos = save;
            this._os.endEncapsulation();
        }
    },
    __writeEmptyParams: function()
    {
        if(this._response)
        {
            Debug.assert(this._os.size === Protocol.headerSize + 4); // Reply status position.
            Debug.assert(this._current.encoding !== null); // Encoding for reply is known.
            this._os.writeByte(Protocol.replyOK);
            this._os.writeEmptyEncapsulation(this._current.encoding);
        }
    },
    __writeParamEncaps: function(v, ok)
    {
        if(this._response)
        {
            Debug.assert(this._os.size === Protocol.headerSize + 4); // Reply status position.
            Debug.assert(this._current.encoding !== null); // Encoding for reply is known.
            this._os.writeByte(ok ? Protocol.replyOK : Protocol.replyUserException);
            if(v === null || v.length === 0)
            {
                this._os.writeEmptyEncapsulation(this._current.encoding);
            }
            else
            {
                this._os.writeEncapsulation(v);
            }
        }
    },
    __writeUserException: function(ex, format)
    {
        var os = this.__startWriteParams(format);
        os.writeUserException(ex);
        this.__endWriteParams(false);
    },
    __warning: function(ex)
    {
        Debug.assert(this._instance !== null);

        var s = [];
        s.push("dispatch exception:");
        s.push("\nidentity: " + this._instance.identityToString(this._current.id));
        s.push("\nfacet: " + StringUtil.escapeString(this._current.facet, ""));
        s.push("\noperation: " + this._current.operation);
        if(this._connection !== null)
        {
            for(var p = this._connection.getInfo(); p; p = p.underlying)
            {
                if(p instanceof Ice.IPConnectionInfo)
                {
                    s.push("\nremote host: " + p.remoteAddress + " remote port: " + p.remotePort);
                }
            }
        }
        if(ex.stack)
        {
            s.push("\n");
            s.push(ex.stack);
        }
        this._instance.initializationData().logger.warning(s.join(""));
    },
    __servantLocatorFinished: function()
    {
        Debug.assert(this._locator !== null && this._servant !== null);
        try
        {
            this._locator.finished(this._current, this._servant, this._cookie.value);
            return true;
        }
        catch(ex)
        {
            if(ex instanceof Ice.UserException)
            {
                Debug.assert(this._connection !== null);

                //
                // The operation may have already marshaled a reply; we must overwrite that reply.
                //
                if(this._response)
                {
                    this._os.resize(Protocol.headerSize + 4); // Reply status position.
                    this._os.writeByte(Protocol.replyUserException);
                    this._os.startEncapsulation();
                    this._os.writeUserException(ex);
                    this._os.endEncapsulation();
                    this._connection.sendResponse(this._os, this._compress);
                }
                else
                {
                    this._connection.sendNoResponse();
                }

                this._connection = null;
            }
            else
            {
                this.__handleException(ex);
            }
            return false;
        }
    },
    __handleException: function(ex)
    {
        Debug.assert(this._connection !== null);

        var props = this._instance.initializationData().properties;
        var s;
        if(ex instanceof Ice.RequestFailedException)
        {
            if(ex.id === null)
            {
                ex.id = this._current.id;
            }

            if(ex.facet === null)
            {
                ex.facet = this._current.facet;
            }

            if(ex.operation === null || ex.operation.length === 0)
            {
                ex.operation = this._current.operation;
            }

            if(props.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 1)
            {
                this.__warning(ex);
            }

            if(this._response)
            {
                this._os.resize(Protocol.headerSize + 4); // Reply status position.
                if(ex instanceof Ice.ObjectNotExistException)
                {
                    this._os.writeByte(Protocol.replyObjectNotExist);
                }
                else if(ex instanceof Ice.FacetNotExistException)
                {
                    this._os.writeByte(Protocol.replyFacetNotExist);
                }
                else if(ex instanceof Ice.OperationNotExistException)
                {
                    this._os.writeByte(Protocol.replyOperationNotExist);
                }
                else
                {
                    Debug.assert(false);
                }
                ex.id.__write(this._os);

                //
                // For compatibility with the old FacetPath.
                //
                if(ex.facet === null || ex.facet.length === 0)
                {
                    Ice.StringSeqHelper.write(this._os, null);
                }
                else
                {
                    Ice.StringSeqHelper.write(this._os, [ ex.facet ]);
                }

                this._os.writeString(ex.operation);

                this._connection.sendResponse(this._os, this._compress);
            }
            else
            {
                this._connection.sendNoResponse();
            }
        }
        else if(ex instanceof Ice.UnknownLocalException)
        {
            if(props.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                this.__warning(ex);
            }

            if(this._response)
            {
                this._os.resize(Protocol.headerSize + 4); // Reply status position.
                this._os.writeByte(Protocol.replyUnknownLocalException);
                this._os.writeString(ex.unknown);
                this._connection.sendResponse(this._os, this._compress);
            }
            else
            {
                this._connection.sendNoResponse();
            }
        }
        else if(ex instanceof Ice.UnknownUserException)
        {
            if(props.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                this.__warning(ex);
            }

            if(this._response)
            {
                this._os.resize(Protocol.headerSize + 4); // Reply status position.
                this._os.writeByte(Protocol.replyUnknownUserException);
                this._os.writeString(ex.unknown);
                this._connection.sendResponse(this._os, this._compress);
            }
            else
            {
                this._connection.sendNoResponse();
            }
        }
        else if(ex instanceof Ice.UnknownException)
        {
            if(props.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                this.__warning(ex);
            }

            if(this._response)
            {
                this._os.resize(Protocol.headerSize + 4); // Reply status position.
                this._os.writeByte(Protocol.replyUnknownException);
                this._os.writeString(ex.unknown);
                this._connection.sendResponse(this._os, this._compress);
            }
            else
            {
                this._connection.sendNoResponse();
            }
        }
        else if(ex instanceof Ice.LocalException)
        {
            if(props.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                this.__warning(ex);
            }

            if(this._response)
            {
                this._os.resize(Protocol.headerSize + 4); // Reply status position.
                this._os.writeByte(Protocol.replyUnknownLocalException);
                //this._os.writeString(ex.toString());
                s = [ ex.ice_name() ];
                if(ex.stack)
                {
                    s.push("\n");
                    s.push(ex.stack);
                }
                this._os.writeString(s.join(""));
                this._connection.sendResponse(this._os, this._compress);
            }
            else
            {
                this._connection.sendNoResponse();
            }
        }
        else if(ex instanceof Ice.UserException)
        {
            if(props.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                this.__warning(ex);
            }

            if(this._response)
            {
                this._os.resize(Protocol.headerSize + 4); // Reply status position.
                this._os.writeByte(Protocol.replyUnknownUserException);
                //this._os.writeString(ex.toString());
                s = [ ex.ice_name() ];
                if(ex.stack)
                {
                    s.push("\n");
                    s.push(ex.stack);
                }
                this._os.writeString(s.join(""));
                this._connection.sendResponse(this._os, this._compress);
            }
            else
            {
                this._connection.sendNoResponse();
            }
        }
        else
        {
            if(props.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                this.__warning(ex);
            }

            if(this._response)
            {
                this._os.resize(Protocol.headerSize + 4); // Reply status position.
                this._os.writeByte(Protocol.replyUnknownException);
                //this._os.writeString(ex.toString());
                this._os.writeString(ex.stack ? ex.stack : "");
                this._connection.sendResponse(this._os, this._compress);
            }
            else
            {
                this._connection.sendNoResponse();
            }
        }

        this._connection = null;
    },
    invoke: function(servantManager, stream)
    {
        this._is = stream;

        var start = this._is.pos;

        //
        // Read the current.
        //
        this._current.id.__read(this._is);

        //
        // For compatibility with the old FacetPath.
        //
        var facetPath = Ice.StringSeqHelper.read(this._is);
        if(facetPath.length > 0)
        {
            if(facetPath.length > 1)
            {
                throw new Ice.MarshalException();
            }
            this._current.facet = facetPath[0];
        }
        else
        {
            this._current.facet = "";
        }

        this._current.operation = this._is.readString();
        this._current.mode = Ice.OperationMode.valueOf(this._is.readByte());
        this._current.ctx = new Context();
        var sz = this._is.readSize();
        while(sz-- > 0)
        {
            var first = this._is.readString();
            var second = this._is.readString();
            this._current.ctx.set(first, second);
        }

        //
        // Don't put the code above into the try block below. Exceptions
        // in the code above are considered fatal, and must propagate to
        // the caller of this operation.
        //

        if(servantManager !== null)
        {
            this._servant = servantManager.findServant(this._current.id, this._current.facet);
            if(this._servant === null)
            {
                this._locator = servantManager.findServantLocator(this._current.id.category);
                if(this._locator === null && this._current.id.category.length > 0)
                {
                    this._locator = servantManager.findServantLocator("");
                }

                if(this._locator !== null)
                {
                    try
                    {
                        this._servant = this._locator.locate(this._current, this._cookie);
                    }
                    catch(ex)
                    {
                        if(ex instanceof Ice.UserException)
                        {
                            var encoding = this._is.skipEncapsulation(); // Required for batch requests.

                            if(this._response)
                            {
                                this._os.writeByte(Protocol.replyUserException);
                                this._os.startEncapsulation(encoding, FormatType.DefaultFormat);
                                this._os.writeUserException(ex);
                                this._os.endEncapsulation();
                                this._connection.sendResponse(this._os, this._compress);
                            }
                            else
                            {
                                this._connection.sendNoResponse();
                            }

                            this._connection = null;
                            return;
                        }
                        else
                        {
                            this._is.skipEncapsulation(); // Required for batch requests.
                            this.__handleException(ex);
                            return;
                        }
                    }
                }
            }
        }

        try
        {
            if(this._servant !== null)
            {
                //
                // DispatchAsync is a "pseudo dispatch status", used internally only
                // to indicate async dispatch.
                //
                if(this._servant.__dispatch(this, this._current) === Ice.DispatchStatus.DispatchAsync)
                {
                    //
                    // If this was an asynchronous dispatch, we're done here.
                    //
                    return;
                }

                if(this._locator !== null && !this.__servantLocatorFinished())
                {
                    return;
                }
            }
            else
            {
                //
                // Skip the input parameters, this is required for reading
                // the next batch request if dispatching batch requests.
                //
                this._is.skipEncapsulation();

                if(servantManager !== null && servantManager.hasServant(this._current.id))
                {
                    throw new Ice.FacetNotExistException(this._current.id, this._current.facet,
                                                            this._current.operation);
                }
                else
                {
                    throw new Ice.ObjectNotExistException(this._current.id, this._current.facet,
                                                            this._current.operation);
                }
            }
        }
        catch(ex)
        {
            if(this._servant !== null && this._locator !== null && !this.__servantLocatorFinished())
            {
                return;
            }
            this.__handleException(ex);
            return;
        }

        //
        // Don't put the code below into the try block above. Exceptions
        // in the code below are considered fatal, and must propagate to
        // the caller of this operation.
        //

        Debug.assert(this._connection !== null);

        if(this._response)
        {
            this._connection.sendResponse(this._os, this._compress);
        }
        else
        {
            this._connection.sendNoResponse();
        }

        this._connection = null;
    },
    startReadParams: function()
    {
        //
        // Remember the encoding used by the input parameters, we'll
        // encode the response parameters with the same encoding.
        //
        this._current.encoding = this._is.startEncapsulation();
        return this._is;
    },
    endReadParams: function()
    {
        this._is.endEncapsulation();
    },
    readEmptyParams: function()
    {
        this._current.encoding = this._is.skipEmptyEncapsulation();
    },
    readParamEncaps: function()
    {
        this._current.encoding = new Ice.EncodingVersion();
        return this._is.readEncapsulation(this._current.encoding);
    },
    __response: function()
    {
        try
        {
            if(this._locator !== null && !this.__servantLocatorFinished())
            {
                return;
            }

            Debug.assert(this._connection !== null);

            if(this._response)
            {
                this._connection.sendResponse(this._os, this._compress);
            }
            else
            {
                this._connection.sendNoResponse();
            }

            this._connection = null;
        }
        catch(ex)
        {
            this._connection.invokeException(ex, 1);
        }
    },
    __exception: function(exc)
    {
        try
        {
            if(this._locator !== null && !this.__servantLocatorFinished())
            {
                return;
            }

            this.__handleException(exc);
        }
        catch(ex)
        {
            this._connection.invokeException(ex, 1);
        }
    },
    __validateResponse: function(ok)
    {
        if(!this._active)
        {
            return false;
        }
        this._active = false;
        return true;
    },
    ice_exception: function(ex)
    {
        if(!this._active)
        {
            return;
        }
        this._active = false;

        if(this._connection !== null)
        {
            this.__exception(ex);
        }
        else
        {
            //
            // Response has already been sent.
            //
            if(this._instance.initializationData().properties.getPropertyAsIntWithDefault("Ice.Warn.Dispatch", 1) > 0)
            {
                this.__warning(ex);
            }
        }
    }
});

Ice.IncomingAsync = IncomingAsync;
module.exports.Ice = Ice;
