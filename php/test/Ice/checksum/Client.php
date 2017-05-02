<?php
// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

error_reporting(E_ALL | E_STRICT);

if(!extension_loaded("ice"))
{
    echo "\nerror: Ice extension is not loaded.\n\n";
    exit(1);
}

$NS = function_exists("Ice\\initialize");
require_once('Ice.php');
require_once('Test.php');
require_once('CTypes.php');

function test($b)
{
    if(!$b)
    {
        $bt = debug_backtrace();
        echo "\ntest failed in ".$bt[0]["file"]." line ".$bt[0]["line"]."\n";
        exit(1);
    }
}

require_once($NS ? "Checksums_ns.php" : "Checksums_no_ns.php");

$communicator = $NS ? eval("return Ice\\initialize(\$argv);") :
                      eval("return Ice_initialize(\$argv);");

$checksum = allTests($communicator);
$checksum->shutdown();
$communicator->destroy();
exit();
?>
