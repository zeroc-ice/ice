//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ReferenceMode").Ice;
const ReferenceMode = Ice.ReferenceMode;

class ConnectionRequestHandler
{
    constructor(ref, connection)
    {
        this._reference = ref;
        this._response = ref.getMode() == ReferenceMode.ModeTwoway;
        this._connection = connection;
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
        return out.invokeRemote(this._connection, this._response);
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
