//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
