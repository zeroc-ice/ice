// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

module Filesystem {
    exception GenericError {
    	string reason;
    };

    interface Node {
    	["cpp:const"] idempotent string name();
    };

    sequence<string> Lines;

    interface File extends Node {
    	["cpp:const"] idempotent Lines read();
	idempotent void write(Lines text) throws GenericError;
    };

    sequence<Node*> NodeSeq;

    interface Directory extends Node {
    	["cpp:const"] idempotent NodeSeq list();
    };
};
