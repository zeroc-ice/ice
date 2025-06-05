<?php
// Copyright (c) ZeroC, Inc.

require_once('Test.php');

function allTests($helper)
{
    $communicator = $helper->communicator();

    echo "testing equals() for Slice structures... ";
    flush();

    //
    // Define some default values.
    //
    $def_s2 = new Test\S2(
        true,
        98,
        99,
        100,
        101,
        1.0,
        2.0,
        "string",
        array("one", "two", "three"),
        array("abc" => "def"),
        new Test\S1("name"),
        new Test\C(5),
        $communicator->stringToProxy("test"));

    // Compare default-constructed structures.
    test(new Test\S2 == new Test\S2);

    // Change one primitive member at a time.
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

    // String member
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

    // Sequence member
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

    // Dictionary member
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

    // Struct member
    $v1 = clone $def_s2;
    $v1->s = clone $def_s2->s;
    test($v1 == $def_s2);

    $v1 = clone $def_s2;
    $v1->s = new Test\S1("name");
    test($v1 == $def_s2);

    $v1 = clone $def_s2;
    $v1->s = new Test\S1("noname");
    test($v1 != $def_s2);

    $v1 = clone $def_s2;
    $v2 = clone $def_s2;
    $v1->s = null;
    test($v1 != $v2);

    $v1 = clone $def_s2;
    $v2 = clone $def_s2;
    $v2->s = null;
    test($v1 != $v2);

    // Class member
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

    // Proxy member
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
