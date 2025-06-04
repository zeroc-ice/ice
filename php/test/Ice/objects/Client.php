<?php
// Copyright (c) ZeroC, Inc.

require_once('Test.php');
require_once('Forward.php');

class BI extends Test\B
{
    function ice_preUnmarshal()
    {
        $this->preUnmarshalInvoked = true;
    }

    function ice_postUnmarshal()
    {
        $this->postUnmarshalInvoked = true;
    }
}

class CI extends Test\C
{
    function ice_preUnmarshal()
    {
        $this->preUnmarshalInvoked = true;
    }

    function ice_postUnmarshal()
    {
        $this->postUnmarshalInvoked = true;
    }
}

class DI extends Test\D
{
    function ice_preUnmarshal()
    {
        $this->preUnmarshalInvoked = true;
    }

    function ice_postUnmarshal()
    {
        $this->postUnmarshalInvoked = true;
    }
}

class CustomSliceLoader implements Ice\SliceLoader
{
    function newInstance(string $typeId) : ?object
    {
        return match($typeId)
        {
            "::Test::B" => new BI(),
            "::Test::C" => new CI(),
            "::Test::D" => new DI(),
            default => null,
        };
    }
}

function allTests($helper)
{
    $ref = sprintf("initial:%s", $helper->getTestEndpoint());
    $communicator = $helper->communicator();
    $initial = Test\InitialPrxHelper::createProxy($communicator, $ref);

    echo "getting B1... ";
    flush();
    $b1 = $initial->getB1();
    test($b1 != null);
    echo "ok\n";

    echo "getting B2... ";
    flush();
    $b2 = $initial->getB2();
    test($b2 != null);
    echo "ok\n";

    echo "getting C... ";
    flush();
    $c = $initial->getC();
    test($c != null);
    echo "ok\n";

    echo "getting D... ";
    flush();
    $d = $initial->getD();
    test($d != null);
    echo "ok\n";

    echo "checking consistency... ";
    flush();
    test($b1 !== $b2);
    test($b1 !== $c);
    test($b1 !== $d);
    test($b2 !== $c);
    test($b2 !== $d);
    test($c !== $d);
    test($b1->theB != null);
    test($b1->theB === $b1);
    test($b1->theC == null);
    test($b1->theA != null);
    test($b1->theA->theA === $b1->theA);
    test($b1->theA->theB === $b1);
    test($b1->theA->theC != null);
    test($b1->theA->theC->theB === $b1->theA);
    test($b1->preMarshalInvoked);
    test($b1->postUnmarshalInvoked);
    test($b1->theA->preMarshalInvoked);
    test($b1->theA->postUnmarshalInvoked);
    test($b1->theA->theC->preMarshalInvoked);
    test($b1->theA->theC->postUnmarshalInvoked);
    // More tests possible for b2 and d, but I think this is already sufficient.
    test($b2->theA === $b2);
    test($d->theC == null);
    echo "ok\n";

    //
    // Break cyclic dependencies
    //
    $b1->theA->theA = null;
    $b1->theA->theB = null;
    $b1->theA->theC = null;
    $b1->theA = null;
    $b1->theB = null;

    $b2->theA = null;
    $b2->theB->theA = null;
    $b2->theB->theB = null;
    $b2->theC = null;

    $c->theB->theA = null;
    $c->theB->theB->theA = null;
    $c->theB->theB->theB = null;
    $c->theB = null;

    $d->theA->theA->theA = null;
    $d->theA->theA->theB = null;
    $d->theA->theB->theA = null;
    $d->theA->theB->theB = null;
    $d->theB->theA = null;
    $d->theB->theB = null;
    $d->theB->theC = null;

    echo "getting B1, B2, C, and D all at once... ";
    flush();
    $initial->getAll($b1, $b2, $c, $d);
    test($b1 != null);
    test($b2 != null);
    test($c != null);
    test($d != null);
    echo "ok\n";

    echo "checking consistency... ";
    flush();
    test($b1 !== $b2);
    test($b1 !== $c);
    test($b1 !== $d);
    test($b2 !== $c);
    test($b2 !== $d);
    test($c !== $d);
    test($b1->theA === $b2);
    test($b1->theB === $b1);
    test($b1->theC == null);
    test($b2->theA === $b2);
    test($b2->theB === $b1);
    test($b2->theC === $c);
    test($c->theB === $b2);
    test($d->theA === $b1);
    test($d->theB === $b2);
    test($d->theC == null);
    test($d->preMarshalInvoked);
    test($d->postUnmarshalInvoked);
    test($d->theA->preMarshalInvoked);
    test($d->theA->postUnmarshalInvoked);
    test($d->theB->preMarshalInvoked);
    test($d->theB->postUnmarshalInvoked);
    test($d->theB->theC->preMarshalInvoked);
    test($d->theB->theC->postUnmarshalInvoked);
    echo "ok\n";

    //
    // Break cyclic dependencies
    //
    $b1->theA = null;
    $b1->theB = null;
    $b2->theA = null;
    $b2->theB = null;
    $b2->theC = null;
    $c->theB = null;
    $d->theA = null;
    $d->theB = null;

    echo "getting K... ";
    flush();
    $k = $initial->getK();
    test($k->value->data == "l");
    echo "ok\n";

    echo "testing Value as parameter... ";
    flush();
    $v1 = new Test\L();
    $v1->data = "l";
    $v2 = null;
    $v3 = $initial->opValue($v1, $v2);
    test($v2->data == "l");
    test($v3->data == "l");

    $v1 = array(new Test\L());
    $v1[0]->data = "l";
    $v2 = null;
    $v3 = $initial->opValueSeq($v1, $v2);
    test($v2[0]->data == "l");
    test($v3[0]->data == "l");

    $v1 = array("l" => new Test\L());
    $v1["l"]->data = "l";
    $v2 = null;
    $v3 = $initial->opValueMap($v1, $v2);
    test($v2["l"]->data == "l");
    test($v3["l"]->data == "l");

    echo "ok\n";

    echo "getting D1... ";
    flush();
    $d1 = $initial->getD1(new Test\D1(new Test\A1("a1"), new Test\A1("a2"), new Test\A1("a3"), new Test\A1("a4")));
    test($d1->a1->name == "a1");
    test($d1->a2->name == "a2");
    test($d1->a3->name == "a3");
    test($d1->a4->name == "a4");
    echo "ok\n";

    echo "throw EDerived... ";
    flush();
    try
    {
        $initial->throwEDerived();
        test(false);
    }
    catch(Test\EDerived $ex)
    {
        test($ex->a1->name == "a1");
        test($ex->a2->name == "a2");
        test($ex->a3->name == "a3");
        test($ex->a4->name == "a4");
    }
    echo "ok\n";

    echo "setting G... ";
    flush();
    try
    {
        $initial->setG(new Test\G(new Test\S("hello"), "g"));
    }
    catch(Ice\OperationNotExistException $ex)
    {
    }
    echo "ok\n";

    echo "testing sequences... ";
    flush();
    $outS = null;
    $initial->opBaseSeq(array(), $outS);

    $seq = array();
    for($i = 0; $i < 120; $i++)
    {
        $b = new Test\Base();
        $b->str = "b" . $i;
        $b->theS = new Test\S();
        $b->theS->str = "b" . $i;
        $seq[$i] = $b;
    }

    $retS = $initial->opBaseSeq($seq, $outS);
    test($seq == $retS);
    test($seq == $outS);
    $i = 0;
    foreach($retS as $obj)
    {
        test($obj == $seq[$i++]);
    }
    $i = 0;
    foreach($outS as $obj)
    {
        test($obj == $seq[$i++]);
    }
    echo "ok\n";

    echo "testing recursive type... ";
    flush();
    $top = new Test\Recursive();
    $bottom = $top;
    $maxDepth = 10;
    for ($i = 1; $i < $maxDepth; $i++)
    {
        $bottom->v = new Test\Recursive();
        $bottom = $bottom->v;
    }
    $initial->setRecursive($top);

    // Adding one more level would exceed the max class graph depth
    $bottom->v = new Test\Recursive();
    $bottom = $bottom->v;

    try
    {
        $initial->setRecursive($top);
        test(false);
    }
    catch(Exception $ex)
    {
        if($ex instanceof Ice\UnknownLocalException)
        {
            // Expected marshal exception from the server (max class graph depth reached)
        }
        else
        {
            throw $ex;
        }
    }
    echo "ok\n";

    echo "testing compact ID... ";
    flush();
    try
    {
        $r = $initial->getCompact();
        test($r != null);
    }
    catch(Ice\OperationNotExistException $ex)
    {
    }
    echo "ok\n";

    echo "testing marshaled results... ";
    flush();
    $b1 = $initial->getMB();
    test($b1 != null && $b1->theB == $b1);
    $b1 = $initial->getAMDMB();
    test($b1 != null && $b1->theB == $b1);
    echo "ok\n";

    echo "testing UnexpectedObjectException... ";
    flush();
    $ref = sprintf("uoet:%s", $helper->getTestEndpoint());
    $uoet = Test\UnexpectedObjectExceptionTestPrxHelper::createProxy($communicator, $ref);
    try
    {
        $uoet->op();
        test(false);
    }
    catch(Exception $ex)
    {
        if($ex instanceof Ice\MarshalException)
        {
            test(str_contains($ex->getMessage(), "::Test::AlsoEmpty"));
            test(str_contains($ex->getMessage(), "::Test::Empty"));
        }
        else
        {
            throw $ex;
        }
    }
    echo "ok\n";

    echo "testing forward declarations... ";
    $f12 = null;
    $f11 = $initial->opF1(new Test\F1("F11"), $f12);
    test($f11->name == "F11");
    test($f12->name == "F12");

    $f22 = null;
    $ref = sprintf("F21:%s", $helper->getTestEndpoint());
    $f21 = $initial->opF2(Test\F2PrxHelper::createProxy($communicator, $ref), $f22);
    test($f21->ice_getIdentity()->name == "F21");
    $f21->op();
    test($f22->ice_getIdentity()->name == "F22");

    if($initial->hasF3())
    {
        $f32 = null;
        $f31 = $initial->opF3(new Test\F3($f11, $f22), $f32);
        test($f31->f1->name == "F11");
        test($f31->f2->ice_getIdentity()->name = "F21");

        test($f32->f1->name == "F12");
        test($f32->f2->ice_getIdentity()->name = "F22");
    }
    echo "ok\n";

    echo "testing sending class cycle...";
    $rec = new Test\Recursive();
    $rec->v = $rec;
    $acceptsCycles = $initial->acceptsClassCycles();
    try
    {
        $initial->setCycle($rec);
        test($acceptsCycles);
    }
    catch(Ice\UnknownLocalException $ex)
    {
        test(!$acceptsCycles);
    }
    echo "ok\n";

    return $initial;
}

class Client extends TestHelper
{
    function run($args)
    {
        $initData = new Ice\InitializationData();
        $initData->properties = $this->createTestProperties($args);
        $initData->sliceLoader = new CustomSliceLoader();
        $communicator = $this->initialize($initData);
        $initial = allTests($this);
        $initial->shutdown();
        $communicator->destroy();
    }
}
