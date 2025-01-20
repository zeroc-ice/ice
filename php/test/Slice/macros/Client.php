<?php
// Copyright (c) ZeroC, Inc.

require_once('Test.php');

function allTests()
{
    global $NS;

    echo "testing Slice predefined macros... ";
    flush();
    $d = new Test\_Default();
    test($d->x == 10);
    test($d->y == 10);

    $nd = new Test\NoDefault();
    test($nd->x != 10);
    test($nd->y != 10);

    $c = new Test\PhpOnly();
    test($c->lang == "php");
    test($c->version == Ice\intVersion());
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
