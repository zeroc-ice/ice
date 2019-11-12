//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;
Ice._ModuleRegistry.require(module,
                            [
                                "../Ice/Connection"
                            ]).Ice;

let IceSSL = Ice._ModuleRegistry.module("IceSSL");

/**
 * Provides access to the connection details of an SSL connection
 *
 **/
IceSSL.ConnectionInfo = class extends Ice.ConnectionInfo
{
    constructor(underlying, incoming, adapterName, connectionId, cipher = "", certs = null, verified = false)
    {
        super(underlying, incoming, adapterName, connectionId);
        this.cipher = cipher;
        this.certs = certs;
        this.verified = verified;
    }
};

exports.IceSSL = IceSSL;
