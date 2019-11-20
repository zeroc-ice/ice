//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;
Ice._ModuleRegistry.require(module,
                            [
                                "../Ice/Endpoint"
                            ]).Ice;

let IceSSL = Ice._ModuleRegistry.module("IceSSL");

/**
 * Provides access to an SSL endpoint information.
 *
 **/
IceSSL.EndpointInfo = class extends Ice.EndpointInfo
{
    constructor(underlying, timeout, compress)
    {
        super(underlying, timeout, compress);
    }
};

exports.IceSSL = IceSSL;
