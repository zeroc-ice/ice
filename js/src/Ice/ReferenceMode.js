// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;

//
// Using a separate module for these constants so that ObjectPrx does
// not need to include Reference.
//
Ice.ReferenceMode = 
{
    ModeTwoway: 0, 
    ModeOneway: 1, 
    ModeBatchOneway: 2, 
    ModeDatagram: 3, 
    ModeBatchDatagram: 4,
    ModeLast: 4
};
module.exports.Ice = Ice;
