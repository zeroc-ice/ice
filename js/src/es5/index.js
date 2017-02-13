// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

require("babel-polyfill")
module.exports.Ice      = require("./Ice/Ice").Ice;
module.exports.IceMX    = require("./Ice/Ice").IceMX;
module.exports.IceSSL   = require("./Ice/Ice").IceSSL;
module.exports.Glacier2 = require("./Glacier2/Glacier2").Glacier2;
module.exports.IceGrid  = require("./IceGrid/IceGrid").IceGrid;
module.exports.IceStorm = require("./IceStorm/IceStorm").IceStorm;
