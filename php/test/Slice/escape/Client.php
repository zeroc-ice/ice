<?php
// Copyright (c) ZeroC, Inc.

require_once('Ice.php');
require_once('Key.php');
require_once('Clash.php');

class echoI extends _and\_echo
{
    public function _else($a, $b)
    {
    }
}

function allTests($helper)
{
    echo "testing type names... ";
    flush();
    $b = new _and\_xor();
    test($b->_abstract == 0);
    test($b->_clone == 0);
    test($b->_private == 0);
    test($b->_protected == 0);
    test($b->_public == 0);
    test($b->_this == 0);
    test($b->_throw == 0);
    test($b->_use == 0);
    test($b->_var == 0);
    $communicator = $helper->communicator();
    $c = _and\functionPrxHelper::createProxy($communicator, "test:tcp -p 10000");
    $d = _and\diePrxHelper::uncheckedCast($c);
    $e1 = new echoI();
    $f = _and\enddeclarePrxHelper::uncheckedCast($c);
    $g = new _and\_endif();
    $h = new _and\_endwhile();
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
