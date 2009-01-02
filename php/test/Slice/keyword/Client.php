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
Ice_loadProfileWithArgs($argv);

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
    function _else($a, &$b)
    {
    }
}

class enddeclareI extends and_enddeclare
{
    function _else($a, &$b)
    {
    }
    function _continue($a, $b)
    {
    }
    function _do()
    {
    }
}

function allTests()
{
    global $ICE;

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
    $c = $ICE->stringToProxy("test:tcp -p 10000")->ice_uncheckedCast("::and::function");
    $d = $ICE->stringToProxy("test:tcp -p 10000")->ice_uncheckedCast("::and::die");
    $e = $ICE->stringToProxy("test:tcp -p 10000")->ice_uncheckedCast("::and::echo");
    $e1 = new echoI();
    $f = $ICE->stringToProxy("test:tcp -p 10000")->ice_uncheckedCast("::and::enddeclare");
    $f1 = new enddeclareI();
    $g = new and_endif();
    $h = new and_endwhile();
    $i = constant("and_or");
    $j = constant("and_print");
    echo "ok\n";
}

allTests();

exit();
?>
