// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
    	nonmutating string name();
    };

    sequence<string> Lines;

    interface File extends Node {
    	nonmutating Lines read();
	idempotent void write(Lines text) throws GenericError;
    };

    sequence<Node*> NodeSeq;

    interface Directory extends Node {
    	nonmutating NodeSeq list();
    };
};
