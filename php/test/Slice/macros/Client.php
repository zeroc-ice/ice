<?php
// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

require_once('Test.php');

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

class Client extends TestHelper
{
    function run($args)
    {
        allTests();
    }
}
?>
