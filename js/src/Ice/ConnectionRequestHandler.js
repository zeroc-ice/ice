// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ReferenceMode").Ice;
const ReferenceMode = Ice.ReferenceMode;

class ConnectionRequestHandler
{
    constructor(ref, connection, compress)
    {
        this._reference = ref;
        this._response = ref.getMode() == ReferenceMode.ModeTwoway;
        this._connection = connection;
        this._compress = compress;
    }

    update(previousHandler, newHandler)
    {
        try
        {
            if(previousHandler === this)
            {
                return newHandler;
            }
            else if(previousHandler.getConnection() === this._connection)
            {
                //
                // If both request handlers point to the same connection, we also
                // update the request handler. See bug ICE-5489 for reasons why
                // this can be useful.
                //
                return newHandler;
            }
        }
        catch(ex)
        {
            // Ignore
        }
        return this;
    }

    sendAsyncRequest(out)
    {
        return out.__invokeRemote(this._connection, this._compress, this._response);
    }

    asyncRequestCanceled(out)
    {
        return this._connection.asyncRequestCanceled(out);
    }

    getReference()
    {
        return this._reference;
    }

    getConnection()
    {
        return this._connection;
    }
}

Ice.ConnectionRequestHandler = ConnectionRequestHandler;
module.exports.Ice = Ice;
