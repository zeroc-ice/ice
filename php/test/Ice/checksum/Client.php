<?php
// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

require_once('Test.php');
require_once('CTypes.php');
global $NS;
require_once($NS ? "Checksums_ns.php" : "Checksums_no_ns.php");

class Client extends TestHelper
{
    function run($args)
    {
        try
        {
            $communicator = $this->initialize($args);
            $checksum = allTests($this);
            $checksum->shutdown();
            $communicator->destroy();
        }
        catch(Exception $ex)
        {
            $communicator->destroy();
            throw $ex;
        }
    }
}
?>
