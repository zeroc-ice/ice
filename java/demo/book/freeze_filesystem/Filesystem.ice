// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

module Filesystem
{
    exception GenericError
    {
        string reason;
    };
    exception PermissionDenied extends GenericError {};
    exception NameInUse extends GenericError {};
    exception IllegalName extends GenericError {};
    exception NoSuchName extends GenericError {};

    interface Node
    {
        idempotent string name();

        ["freeze:write"]
        void destroy()
            throws PermissionDenied;
    };

    sequence<string> Lines;

    interface File extends Node
    {
        idempotent Lines read();

        ["freeze:write"]
        idempotent void write(Lines text)
            throws GenericError;
    };

    enum NodeType { DirType, FileType };

    struct NodeDesc
    {
        string name;
        NodeType type;
        Node* proxy;
    };

    dictionary<string, NodeDesc> NodeDict;

    enum ListMode { NormalList, RecursiveList };

    interface Directory extends Node
    {
        idempotent NodeDict list(ListMode mode);

        idempotent NodeDesc resolve(string path)
            throws NoSuchName;

        ["freeze:write"]
        File* createFile(string name)
            throws NameInUse, IllegalName;

        ["freeze:write"]
        Directory* createDirectory(string name)
            throws NameInUse, IllegalName;
    };
};
