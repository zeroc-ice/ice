<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

class Client extends TestHelper
{
    function run($args)
    {
        $properties = Ice\getProperties();
        test($properties->getPropertyAsInt("Ice.Trace.Protocol") == 1);
        test($properties->getPropertyAsInt("Ice.Trace.Network") == 1);
        test($properties->getPropertyAsInt("Ice.Warn.Connections") == 1);
        test($properties->getProperty("Hello.Proxy") == "hello:tcp -h localhost -p 10000");
    }
}

?>
