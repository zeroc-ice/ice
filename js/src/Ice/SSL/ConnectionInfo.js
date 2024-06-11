//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { ConnectionInfo as IceConnectionInfo } from "../Connection.js";

/**
 *  Provides access to the connection details of an SSL connection
 **/
export class ConnectionInfo extends IceConnectionInfo
{
    constructor(underlying, incoming, adapterName, connectionId, cipher = "", certs = null, verified = false)
    {
        super(underlying, incoming, adapterName, connectionId);
        this.cipher = cipher;
        this.certs = certs;
        this.verified = verified;
    }
}
