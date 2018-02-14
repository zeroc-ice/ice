<?
// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
require_once ($NS ? 'Ice_ns.php' : 'Ice.php');
require_once 'Test.php';

function test($b)
{
    if(!$b)
    {
        $bt = debug_backtrace();
        echo "\ntest failed in ".$bt[0]["file"]." line ".$bt[0]["line"]."\n";
        exit(1);
    }
}

function allTests($communicator)
{
    global $NS;

    echo "testing equals() for Slice structures... ";
    flush();

    //
    // Define some default values.
    //
    $def_s2 = $NS ?
        eval("return new Test\\S2(true, 98, 99, 100, 101, 1.0, 2.0, \"string\", array(\"one\", \"two\", \"three\"),
                                  array(\"abc\" => \"def\"), new Test\\S1(\"name\"), new Test\\C(5),
                                  \$communicator->stringToProxy(\"test\"));") :
        new Test_S2(true, 98, 99, 100, 101, 1.0, 2.0, "string", array("one", "two", "three"),
                    array("abc" => "def"), new Test_S1("name"), new Test_C(5),
                    $communicator->stringToProxy("test"));

    //
    // Compare default-constructed structures.
    //
    test($NS ? eval("return new Test\\S2 == new Test\\S2;") : new Test_S2 == new Test_S2);

    //
    // Change one primitive member at a time.
    //
    $v = clone $def_s2;
    test($v == $def_s2);

    $v = clone $def_s2;
    $v->bo = false;
    test($v != $def_s2);

    $v = clone $def_s2;
    $v->by = $v->by - 1;
    test($v != $def_s2);

    $v = clone $def_s2;
    $v->sh = $v->sh - 1;
    test($v != $def_s2);

    $v = clone $def_s2;
    $v->i = $v->i - 1;
    test($v != $def_s2);

    $v = clone $def_s2;
    $v->l = $v->l - 1;
    test($v != $def_s2);

    $v = clone $def_s2;
    $v->f = $v->f - 1;
    test($v != $def_s2);

    $v = clone $def_s2;
    $v->d = $v->d - 1;
    test($v != $def_s2);

    $v = clone $def_s2;
    $v->str = "";
    test($v != $def_s2);

    //
    // String member
    //
    $v1 = clone $def_s2;
    $v1->str = "string";
    test($v1 == $def_s2);

    $v1 = clone $def_s2;
    $v2 = clone $def_s2;
    $v1->str = null;
    test($v1 != $v2);

    $v1 = clone $def_s2;
    $v2 = clone $def_s2;
    $v2->str = null;
    test($v1 != $v2);

    $v1 = clone $def_s2;
    $v2 = clone $def_s2;
    $v1->str = null;
    $v2->str = null;
    test($v1 == $v2);

    //
    // Sequence member
    //
    $v1 = clone $def_s2;
    $v1->ss = $def_s2->ss;
    test($v1 == $def_s2);

    $v1 = clone $def_s2;
    $v1->ss = array();
    test($v1 != $def_s2);

    $v1 = clone $def_s2;
    $v1->ss = array("one", "two", "three");
    test($v1 == $def_s2);

    $v1 = clone $def_s2;
    $v2 = clone $def_s2;
    $v1->ss = null;
    test($v1 != $v2);

    $v1 = clone $def_s2;
    $v2 = clone $def_s2;
    $v2->ss = null;
    test($v1 != $v2);

    //
    // Dictionary member
    //
    $v1 = clone $def_s2;
    $v1->sd = array("abc"=>"def");
    test($v1 == $def_s2);

    $v1 = clone $def_s2;
    $v1->sd = array();
    test($v1 != $def_s2);

    $v1 = clone $def_s2;
    $v2 = clone $def_s2;
    $v1->sd = null;
    test($v1 != $v2);

    $v1 = clone $def_s2;
    $v2 = clone $def_s2;
    $v2->sd = null;
    test($v1 != $v2);

    //
    // Struct member
    //
    $v1 = clone $def_s2;
    $v1->s = clone $def_s2->s;
    test($v1 == $def_s2);

    $v1 = clone $def_s2;
    $v1->s = $NS ? eval("return new Test\\S1(\"name\");") : new Test_S1("name");
    test($v1 == $def_s2);

    $v1 = clone $def_s2;
    $v1->s = $NS ? eval("return new Test\\S1(\"noname\");") : new Test_S1("noname");
    test($v1 != $def_s2);

    $v1 = clone $def_s2;
    $v2 = clone $def_s2;
    $v1->s = null;
    test($v1 != $v2);

    $v1 = clone $def_s2;
    $v2 = clone $def_s2;
    $v2->s = null;
    test($v1 != $v2);

    //
    // Class member
    //
    $v1 = clone $def_s2;
    $v1->cls = clone $def_s2->cls;
    test($v1 == $def_s2); // PHP performs its own equality test.

    $v1 = clone $def_s2;
    $v2 = clone $def_s2;
    $v1->cls = null;
    test($v1 != $v2);

    $v1 = clone $def_s2;
    $v2 = clone $def_s2;
    $v2->cls = null;
    test($v1 != $v2);

    //
    // Proxy member
    //
    $v1 = clone $def_s2;
    $v1->prx = $communicator->stringToProxy("test");
    test($v1 == $def_s2);

    $v1 = clone $def_s2;
    $v1->prx = $communicator->stringToProxy("test2");
    test($v1 != $def_s2);

    $v1 = clone $def_s2;
    $v2 = clone $def_s2;
    $v1->prx = null;
    test($v1 != $v2);

    $v1 = clone $def_s2;
    $v2 = clone $def_s2;
    $v2->prx = null;
    test($v1 != $v2);

    echo "ok\n";
}

$communicator = Ice_initialize($argv);
allTests($communicator);
$communicator->destroy();

exit();
?>
