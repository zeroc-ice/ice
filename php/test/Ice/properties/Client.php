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

        echo "testing ice properties with set default values...";
        $properties = Ice\createProperties();

        $toStringMode = $properties->getIceProperty("Ice.ToStringMode");
        test($toStringMode == "Unicode");

        $closeTimeout = $properties->getIcePropertyAsInt("Ice.Connection.Client.CloseTimeout");
        test($closeTimeout == 10);

        $retryIntervals = $properties->getIcePropertyAsList("Ice.RetryIntervals");
        test($retryIntervals == ["0"]);

        echo "ok\n";

        echo "testing ice properties with unset default values...";
        $properties = Ice\createProperties();

        $stringValue = $properties->getIceProperty("Ice.Admin.Router");
        test($stringValue == "");

        $intValue = $properties->getIcePropertyAsInt("Ice.Admin.Router");
        test($intValue == 0);

        $listValue = $properties->getIcePropertyAsList("Ice.Admin.Router");
        test($listValue == []);

        echo "ok\n";

        echo "testing load properties exception... ";
        try {
            $properties = Ice\createProperties();
            $properties->load("./config/xxxx.config");
            test(False);
        } catch (\Ice\LocalException $ex) {
            test(str_contains($ex->getMessage(), "error while accessing file './config/xxxx.config'"));
        }
        echo "ok\n";

        echo "testing that getting an unknown ice property throws an exception...";
        try {
            $properties = Ice\createProperties();
            $properties->getIceProperty("Ice.UnknownProperty");
            test(False);
        } catch (\Ice\PropertyException $ex) {
            test($ex->getMessage() == "unknown Ice property: Ice.UnknownProperty");
        }
        echo "ok\n";
    }
}
