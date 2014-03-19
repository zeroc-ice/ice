// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    var Ice = global.Ice || {};
    
    //
    // Using a separate module for these constants so that ObjectPrx does
    // not need to include Reference.
    //
    var ReferenceMode = 
    {
        ModeTwoway: 0, 
        ModeOneway: 1, 
        ModeBatchOneway: 2, 
        ModeDatagram: 3, 
        ModeBatchDatagram: 4,
        ModeLast: 4
    };

    Ice.ReferenceMode = ReferenceMode;
    
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
