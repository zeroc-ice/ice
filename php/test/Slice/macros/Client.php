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

function test($b)
{
    if(!$b)
    {
        $bt = debug_backtrace();
        echo "\ntest failed in ".$bt[0]["file"]." line ".$bt[0]["line"]."\n";
        exit(1);
    }
}

function allTests()
{
    global $NS;

    echo "testing Slice predefined macros... ";
    flush();

    if($NS)
    {
        $d = new Test\_Default();
        test($d->x == 10);
        test($d->y == 10);

        $nd = new Test\NoDefault();
        test($nd->x != 10);
        test($nd->y != 10);

        $c = new Test\PhpOnly();
        test($c->lang == "php");
        test($c->version == Ice\intVersion());
    }
    else
    {
        $d = new Test_Default();
        test($d->x == 10);
        test($d->y == 10);

        $nd = new Test_NoDefault();
        test($nd->x != 10);
        test($nd->y != 10);

        $c = new Test_PhpOnly();
        test($c->lang == "php");
        test($c->version == Ice_intVersion());
    }
    echo "ok\n";
}

allTests();

exit();
?>
