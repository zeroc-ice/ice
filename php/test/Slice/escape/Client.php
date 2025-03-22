<?php
// Copyright (c) ZeroC, Inc.

require_once('Ice.php');
require_once('Key.php');
require_once('Clash.php');

function allTests($helper)
{
    echo "testing type names... ";
    flush();

    test(escapedAnd\another_name == 1);
    $b = new escapedAnd\_xor();
    test($b->_abstract == 0);
    test($b->_var == 1);
    $communicator = $helper->communicator();
    $c = escapedAnd\_functionPrxHelper::createProxy($communicator, "test:tcp -p 10000");
    $d = escapedAnd\_diePrxHelper::uncheckedCast($c);
    $e1 = new _echoI();
    $f = escapedAnd\_enddeclarePrxHelper::uncheckedCast($c);
    $g = new escapedAnd\_endif();
    $h = new escapedAnd\_endwhile();
    echo "ok\n";
}

class Client extends TestHelper
{
    function run($args)
    {
        try
        {
            $communicator = $this->initialize($args);
            allTests($this);
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
