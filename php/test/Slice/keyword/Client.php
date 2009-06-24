<?
// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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

require 'Ice.php';
require 'Key.php';

function test($b)
{
    if(!$b)
    {
        $bt = debug_backtrace();
        die("\ntest failed in ".$bt[0]["file"]." line ".$bt[0]["line"]."\n");
    }
}

class echoI extends and_echo
{
    public function _else($a, $b)
    {
    }
}

class enddeclareI extends and_enddeclare
{
    function _else($a, $b)
    {
    }

    function _continue($a, $b)
    {
    }

    function _do()
    {
    }
}

function allTests($communicator)
{
    echo "testing type names... ";
    flush();
    $a = and_array::_as;
    $b = new and_xor();
    test($b->_abstract == 0);
    test($b->_clone == 0);
    test($b->_private == 0);
    test($b->_protected == 0);
    test($b->_public == 0);
    test($b->_this == 0);
    test($b->_throw == 0);
    test($b->_use == 0);
    test($b->_var == 0);
    $p = $communicator->stringToProxy("test:tcp -p 10000");
    $c = and_functionPrxHelper::uncheckedCast($p);
    $d = and_diePrxHelper::uncheckedCast($p);
    $e = and_echoPrxHelper::uncheckedCast($p);
    $e1 = new echoI();
    $f = and_enddeclarePrxHelper::uncheckedCast($p);
    $f1 = new enddeclareI();
    $g = new and_endif();
    $h = new and_endwhile();
    $i = constant("and_or");
    $j = constant("and_print");
    echo "ok\n";
}

$communicator = Ice_initialize(&$argv);
allTests($communicator);
$communicator->destroy();

exit();
?>
