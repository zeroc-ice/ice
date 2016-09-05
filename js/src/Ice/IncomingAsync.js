// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Stream",
        "../Ice/BuiltinSequences",
        "../Ice/Connection",
        "../Ice/Current",
        "../Ice/Debug",
        "../Ice/Exception",
        "../Ice/Identity",
        "../Ice/LocalException",
        "../Ice/Protocol",
        "../Ice/StringUtil"
    ]);

const OutputStream = Ice.OutputStream;
const Current = Ice.Current;
const Debug = Ice.Debug;
const FormatType = Ice.FormatType;
const Context = Ice.Context;
const Identity = Ice.Identity;
const Protocol = Ice.Protocol;
const StringUtil = Ice.StringUtil;

class IncomingAsync
{
    constructor(instance, connection, adapter, response, compress, requestId)
    {
        this._instance = instance;
        this._response = response;
        this._compress = compress;
        this._connection = connection;
        this._format = Ice.FormatType.DefaultFormat;

        this._current = new Current();
        this._current.id = new Identity();
        this._current.adapter = adapter;
        this._current.con = this._connection;
        this._current.requestId = requestId;

        this._servant = null;
        this._locator = null;
        this._cookie = { value: null };

        this._os = null;
        this._is = null;
    }

    startWriteParams()
    {
        if(!this._response)
        {
            throw new Ice.MarshalException("can't marshal out parameters for oneway dispatch");
        }

        Debug.assert(this._current.encoding !== null); // Encoding for reply is known.
        this._os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
        this._os.writeBlob(Protocol.replyHdr);
        this._os.writeInt(this._current.requestId);
        this._os.writeByte(0);
        this._os.startEncapsulation(this._current.encoding, this._format);
        return this._os;
    }

    endWriteParams()
    {
        if(this._response)
        {
            this._os.endEncapsulation();
        }
    }

    writeEmptyParams()
    {
        if(this._response)
        {
            Debug.assert(this._current.encoding !== null); // Encoding for reply is known.
            this._os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
            this._os.writeBlob(Protocol.replyHdr);
            this._os.writeInt(this._current.requestId);
            this._os.writeByte(Protocol.replyOK);
            this._os.writeEmptyEncapsulation(this._current.encoding);
        }
    }

    writeParamEncaps(v, ok)
    {
        if(this._response)
        {
            Debug.assert(this._current.encoding !== null); // Encoding for reply is known.
            this._os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
            this._os.writeBlob(Protocol.replyHdr);
            this._os.writeInt(this._current.requestId);
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
    }

    setFormat(format)
    {
        this._format = format;
    }

    warning(ex)
    {
        Debug.assert(this._instance !== null);

        const s = [];
        s.push("dispatch exception:");
        s.push("\nidentity: " + Ice.identityToString(this._current.id));
        s.push("\nfacet: " + StringUtil.escapeString(this._current.facet, ""));
        s.push("\noperation: " + this._current.operation);
        if(this._connection !== null)
        {
            for(let p = this._connection.getInfo(); p; p = p.underlying)
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
    }

    servantLocatorFinished()
    {
        Debug.assert(this._locator !== null && this._servant !== null);
        try
        {
            this._locator.finished(this._current, this._servant, this._cookie.value);
            return true;
        }
        catch(ex)
        {
            this.handleException(ex);
        }
        return false;
    }

    handleException(ex)
    {
        Debug.assert(this._connection !== null);

        const props = this._instance.initializationData().properties;
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
                this.warning(ex);
            }

            if(this._response)
            {
                this._os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
                this._os.writeBlob(Protocol.replyHdr);
                this._os.writeInt(this._current.requestId);
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
                this.warning(ex);
            }

            if(this._response)
            {
                this._os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
                this._os.writeBlob(Protocol.replyHdr);
                this._os.writeInt(this._current.requestId);
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
                this.warning(ex);
            }

            if(this._response)
            {
                this._os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
                this._os.writeBlob(Protocol.replyHdr);
                this._os.writeInt(this._current.requestId);
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
                this.warning(ex);
            }

            if(this._response)
            {
                this._os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
                this._os.writeBlob(Protocol.replyHdr);
                this._os.writeInt(this._current.requestId);
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
                this.warning(ex);
            }

            if(this._response)
            {
                this._os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
                this._os.writeBlob(Protocol.replyHdr);
                this._os.writeInt(this._current.requestId);
                this._os.writeByte(Protocol.replyUnknownLocalException);
                //this._os.writeString(ex.toString());
                let s = [ ex.ice_name() ];
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
            if(this._response)
            {
                this._os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
                this._os.writeBlob(Protocol.replyHdr);
                this._os.writeInt(this._current.requestId);
                this._os.writeByte(Protocol.replyUserException);
                this._os.startEncapsulation(this._current.encoding, this._format);
                this._os.writeUserException(ex);
                this._os.endEncapsulation();
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
                this.warning(ex);
            }

            if(this._response)
            {
                this._os = new OutputStream(this._instance, Protocol.currentProtocolEncoding);
                this._os.writeBlob(Protocol.replyHdr);
                this._os.writeInt(this._current.requestId);
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
    }

    invoke(servantManager, stream)
    {
        this._is = stream;

        //
        // Read the current.
        //
        this._current.id.__read(this._is);

        //
        // For compatibility with the old FacetPath.
        //
        const facetPath = Ice.StringSeqHelper.read(this._is);
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
        let sz = this._is.readSize();
        while(sz-- > 0)
        {
            this._current.ctx.set(this._is.readString(), this._is.readString());
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
                        this.skipReadParams(); // Required for batch requests.
                        this.handleException(ex);
                        return;
                    }
                }
            }
        }

        if(this._servant === null)
        {
            try
            {
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
            catch(ex)
            {
                this.skipReadParams(); // Required for batch requests.
                this.handleException(ex);
                return;
            }
        }

        try
        {
            Debug.assert(this._servant !== null);

            let promise = this._servant.__dispatch(this, this._current);
            if(promise !== null)
            {
                promise.then(() => this.response(), (ex) => this.exception(ex));
                return;
            }

            Debug.assert(!this._response || this._os !== null);
            this.response();
        }
        catch(ex)
        {
            this.exception(ex);
        }
    }

    startReadParams()
    {
        //
        // Remember the encoding used by the input parameters, we'll
        // encode the response parameters with the same encoding.
        //
        this._current.encoding = this._is.startEncapsulation();
        return this._is;
    }

    endReadParams()
    {
        this._is.endEncapsulation();
    }

    readEmptyParams()
    {
        this._current.encoding = this._is.skipEmptyEncapsulation();
    }

    readParamEncaps()
    {
        this._current.encoding = new Ice.EncodingVersion();
        return this._is.readEncapsulation(this._current.encoding);
    }

    skipReadParams()
    {
        this._current.encoding = this._is.skipEncapsulation();
    }

    response()
    {
        try
        {
            if(this._locator !== null && !this.servantLocatorFinished())
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
        }
        catch(ex)
        {
            console.log(ex);
            this._connection.invokeException(ex, 1);
        }
        this._connection = null;
    }

    exception(exc)
    {
        try
        {
            if(this._locator !== null && !this.servantLocatorFinished())
            {
                return;
            }
            this.handleException(exc);
        }
        catch(ex)
        {
            this._connection.invokeException(ex, 1);
        }
    }
}

Ice.IncomingAsync = IncomingAsync;
module.exports.Ice = Ice;
