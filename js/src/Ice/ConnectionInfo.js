//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

/* eslint-disable */
/* jshint ignore: start */

/* slice2js browser-bundle-skip */
const _ModuleRegistry = require("../Ice/ModuleRegistry").Ice._ModuleRegistry;
require("../Ice/Object");
require("../Ice/Value");
require("../Ice/ObjectPrx");
require("../Ice/Long");
require("../Ice/HashMap");
require("../Ice/HashUtil");
require("../Ice/ArrayUtil");
require("../Ice/StreamHelpers");
require("../Ice/Connection");
const Ice = _ModuleRegistry.module("Ice");

const Slice = Ice.Slice;
/* slice2js browser-bundle-skip-end */
/* slice2js browser-bundle-skip */

let IceSSL = _ModuleRegistry.module("IceSSL");
/* slice2js browser-bundle-skip-end */

/**
 *  Provides access to the connection details of an SSL connection
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

/* slice2js browser-bundle-skip */
exports.IceSSL = IceSSL;
/* slice2js browser-bundle-skip-end */
