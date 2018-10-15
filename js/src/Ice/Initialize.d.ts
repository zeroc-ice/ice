// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

declare module "ice"
{
    namespace Ice
    {
        class InitializationData
        {
            constructor();
            clone():InitializationData;

            properties:Properties;
            logger:Logger;
            valueFactoryManager:ValueFactoryManager;
        }

        function initialize(initData?:InitializationData):Communicator;
        function initialize(args:string[], initData?:InitializationData):Communicator;

        function createProperties(args?:string[], defaults?:Properties):Properties;

        function currentProtocol():ProtocolVersion;
        function currentEncoding():EncodingVersion;
        function stringVersion():string;
        function intVersion():number;
    }
}
