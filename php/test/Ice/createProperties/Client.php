<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

class Client extends TestHelper
{
    function run($args)
    {
        echo "testing create properties with no arguments... ";
        $properties = Ice\createProperties();
        echo "ok\n";

        echo "testing create properties with args... ";
        $properties = Ice\createProperties(array("--Ice.Trace.Network=3"));
        test($properties->getProperty("Ice.Trace.Network"), "3");
        echo "ok\n";

        echo "testing create properties with defaults... ";
        $defaults = Ice\createProperties(array("--Ice.Trace.Network=3", "--Ice.Trace.Protocol=1"));
        $properties = Ice\createProperties(array("--Ice.Trace.Network=1"), $defaults);
        test($properties->getProperty("Ice.Trace.Network"), "1");
        test($properties->getProperty("Ice.Trace.Protocol"), "1");
        echo "ok\n";
    }
}

?>
