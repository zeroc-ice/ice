//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

require("../Ice/ModuleRegistry");
require("../Ice/Initialize");
require("../Ice/Communicator");
require("../Ice/CommunicatorI");
require("../Ice/HashMap");
require("../Ice/Object");
require("../Ice/Value");
require("../Ice/Long");
require("../Ice/Logger");
require("../Ice/LoggerI");
require("../Ice/ObjectPrx");
require("../Ice/BatchRequestQueue");
require("../Ice/Properties");
require("../Ice/PropertiesI");
require("../Ice/IdentityUtil");
require("../Ice/ProcessLogger");
require("../Ice/Protocol");
require("../Ice/Identity");
require("../Ice/Exception");
require("../Ice/LocalException");
require("../Ice/BuiltinSequences");
require("../Ice/StreamHelpers");
require("../Ice/Promise");
require("../Ice/EndpointTypes");
require("../Ice/Locator");
require("../Ice/Router");
require("../Ice/Version");
require("../Ice/Buffer");
require("../Ice/ArrayUtil");
require("../Ice/UnknownSlicedValue");
require("../Ice/Process");
require("../Ice/MapUtil");
require("../Ice/ToStringMode");

module.exports.Ice = require("../Ice/ModuleRegistry").Ice;
module.exports.IceMX = require("../Ice/Metrics").IceMX;
module.exports.IceSSL = require("../Ice/EndpointInfo").IceSSL;
