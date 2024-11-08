<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

class Client extends TestHelper
{
    function run($args)
    {
        $properties = Ice\getProperties();
        test($properties->getIcePropertyAsInt("Ice.Trace.Protocol") == 1);
        test($properties->getIcePropertyAsInt("Ice.Trace.Network") == 1);
        test($properties->getIcePropertyAsInt("Ice.Warn.Connections") == 1);
        test($properties->getProperty("Hello.Proxy") == "hello:tcp -h localhost -p 10000");
    }
}
