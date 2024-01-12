<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
    $p = $communicator->stringToProxy("test:tcp -p 10000");
    $c = _and\functionPrxHelper::uncheckedCast($p);
    $d = _and\diePrxHelper::uncheckedCast($p);
    $e1 = new echoI();
    $f = _and\enddeclarePrxHelper::uncheckedCast($p);
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
