<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

class Client extends TestHelper
{
    function run($args)
    {
        global $NS;
	echo "testing create properties with no arguments... ";
        $properties = call_user_func($NS ? "\\Ice\\createProperties" : "Ice_createProperties");
	echo "ok\n";

	echo "testing create properties with args... ";
	$properties = call_user_func(
            $NS ? "\\Ice\\createProperties" : "Ice_createProperties",
            array("--Ice.Trace.Network=3"));
        test($properties->getProperty("Ice.Trace.Network"), "3");
	echo "ok\n";


        echo "testing create properties with defaults... ";
        $defaults = call_user_func(
            $NS ? "\\Ice\\createProperties" : "Ice_createProperties",
            array("--Ice.Trace.Network=3", "--Ice.Trace.Protocol=1"));
        $properties = call_user_func_array(
            $NS ? "\\Ice\\createProperties" : "Ice_createProperties",
            array(array("--Ice.Trace.Network=1"), &$defaults));
        test($properties->getProperty("Ice.Trace.Network"), "1");
        test($properties->getProperty("Ice.Trace.Protocol"), "1");
        echo "ok\n";
    }
}

?>
