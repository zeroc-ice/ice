<?php
// Copyright (c) ZeroC, Inc.

require_once('ClientPrivate.php');

function allTests($helper)
{
    global $Ice_Encoding_1_0;

    $communicator = $helper->communicator();
    $test = Test\TestIntfPrxHelper::createProxy($communicator, sprintf("Test:%s", $helper->getTestEndpoint()));

    echo "base as Object... ";
    flush();
    {
        $o = $test->SBaseAsObject();
        test($o != null);
        test(get_class($o) == "Test\\SBase");
        test($o->sb == "SBase.sb");
    }
    echo "ok\n";

    echo "base as base... ";
    flush();
    {
        $o = $test->SBaseAsSBase();
        test($o->sb == "SBase.sb");
    }
    echo "ok\n";

    echo "base with known derived as base... ";
    flush();
    {
        $sb = $test->SBSKnownDerivedAsSBase();
        test($sb->sb == "SBSKnownDerived.sb");
        test(get_class($sb) == "Test\\SBSKnownDerived");
        test($sb->sbskd == "SBSKnownDerived.sbskd");
    }
    echo "ok\n";

    echo "base with known derived as known derived... ";
    flush();
    {
        $sbksd = $test->SBSKnownDerivedAsSBSKnownDerived();
        test($sbksd->sbskd == "SBSKnownDerived.sbskd");
    }
    echo "ok\n";

    echo "base with unknown derived as base... ";
    flush();
    {
        $sb = $test->SBSUnknownDerivedAsSBase();
        test($sb->sb == "SBSUnknownDerived.sb");
    }
    if($test->ice_getEncodingVersion() == $Ice_Encoding_1_0)
    {
        try
        {
            //
            // This test succeeds for the 1.0 encoding.
            //
            $sb = $test->SBSUnknownDerivedAsSBaseCompact();
            test($sb->sb == "SBSUnknownDerived.sb");
        }
        catch(Exception $ex)
        {
            test(false);
        }
    }
    else
    {
        try
        {
            //
            // This test fails when using the compact format because the instance cannot
            // be sliced to a known type.
            //
            $sb = $test->SBSUnknownDerivedAsSBaseCompact();
            test(false);
        }
        catch(Exception $ex)
        {
            test(get_class($ex) == "Ice\\MarshalException");
        }
    }
    echo "ok\n";

    echo "unknown with Object as Object... ";
    flush();
    {
        try
        {
            $o = $test->SUnknownAsObject();
            test($test->ice_getEncodingVersion() != $Ice_Encoding_1_0);
            test($o instanceof Ice\UnknownSlicedValue);
            test($o->ice_id() == "::Test::SUnknown");
            test($o->ice_getSlicedData() != null);
            $test->checkSUnknown($o);
        }
        catch(Ice\MarshalException $b)
        {
            test($test->ice_getEncodingVersion() == $Ice_Encoding_1_0);
        }
    }
    echo "ok\n";

    echo "one-element cycle... ";
    flush();
    {
        $b = $test->oneElementCycle();
        test($b != null);
        test(get_class($b) == "Test\B");
        test($b->sb == "B1.sb");
        test($b->pb === $b); // Object identity comparison

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $b->pb = null;
    }
    echo "ok\n";

    echo "two-element cycle... ";
    flush();
    {
        $b1 = $test->twoElementCycle();
        test($b1 != null);
        test(get_class($b1) == "Test\B");
        test($b1->sb == "B1.sb");

        $b2 = $b1->pb;
        test($b2 != null);
        test(get_class($b2) == "Test\B");
        test($b2->sb == "B2.sb");
        test($b2->pb === $b1); // Object identity comparison

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $b1->pb = null;
    }
    echo "ok\n";

    echo "known derived pointer slicing as base... ";
    flush();
    {
        $b1 = $test->D1AsB();
        test($b1 != null);
        test(get_class($b1) == "Test\D1");
        test($b1->sb == "D1.sb");
        test($b1->pb != null);
        test($b1->pb !== $b1); // Object identity comparison
        $d1 = $b1;
        test($d1->sd1 == "D1.sd1");
        test($d1->pd1 != null);
        test($d1->pd1 !== $b1); // Object identity comparison
        test($b1->pb === $d1->pd1); // Object identity comparison

        $b2 = $b1->pb;
        test($b2 != null);
        test($b2->pb === $b1); // Object identity comparison
        test($b2->sb == "D2.sb");
        test(get_class($b2) == "Test\B");

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $b2->pb = null;
    }
    echo "ok\n";

    echo "known derived pointer slicing as derived... ";
    flush();
    {
        $d1 = $test->D1AsD1();
        test($d1 != null);
        test(get_class($d1) == "Test\D1");
        test($d1->sb == "D1.sb");
        test($d1->pb != null);
        test($d1->pb !== $d1); // Object identity comparison

        $b2 = $d1->pb;
        test($b2 != null);
        test(get_class($b2) == "Test\B");
        test($b2->sb == "D2.sb");
        test($b2->pb === $d1); // Object identity comparison

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $b2->pb = null;
    }
    echo "ok\n";

    echo "unknown derived pointer slicing as base... ";
    flush();
    {
        $b2 = $test->D2AsB();
        test($b2 != null);
        test(get_class($b2) == "Test\B");
        test($b2->sb == "D2.sb");
        test($b2->pb != null);
        test($b2->pb !== $b2); // Object identity comparison

        $b1 = $b2->pb;
        test($b1 != null);
        test(get_class($b1) == "Test\D1");
        test($b1->sb == "D1.sb");
        test($b1->pb === $b2); // Object identity comparison
        $d1 = $b1;
        test($d1 != null);
        test($d1->sd1 == "D1.sd1");
        test($d1->pd1 === $b2); // Object identity comparison

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $b1->pb = null;
        $d1->pd1 = null;
    }
    echo "ok\n";

    echo "parameter pointer slicing with known first... ";
    flush();
    {
        $test->paramTest1($b1, $b2);

        test($b1 != null);
        test(get_class($b1) == "Test\D1");
        test($b1->sb == "D1.sb");
        test($b1->pb === $b2); // Object identity comparison
        $d1 = $b1;
        test($d1 != null);
        test($d1->sd1 == "D1.sd1");
        test($d1->pd1 === $b2); // Object identity comparison

        test($b2 != null);
        test(get_class($b2) == "Test\B"); // No custom Slice loader, must be sliced
        test($b2->sb == "D2.sb");
        test($b2->pb === $b1); // Object identity comparison

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $b2->pb = null;
    }
    echo "ok\n";

    echo "parameter pointer slicing with unknown first... ";
    flush();
    {
        $test->paramTest2($b2, $b1);

        test($b1 != null);
        test(get_class($b1) == "Test\D1");
        test($b1->sb == "D1.sb");
        test($b1->pb === $b2); // Object identity comparison
        $d1 = $b1;
        test($d1 != null);
        test($d1->sd1 == "D1.sd1");
        test($d1->pd1 === $b2); // Object identity comparison

        test($b2 != null);
        test(get_class($b2) == "Test\B"); // No custom Slice loader, must be sliced
        test($b2->sb == "D2.sb");
        test($b2->pb === $b1); // Object identity comparison

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $b2->pb = null;
    }
    echo "ok\n";

    echo "return value identity with known first... ";
    flush();
    {
        $r = $test->returnTest1($p1, $p2);
        test($r === $p1);

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $p2->pb = null;
    }
    echo "ok\n";

    echo "return value identity with unknown first... ";
    flush();
    {
        $r = $test->returnTest2($p1, $p2);
        test($r === $p1);

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $p1->pb = null;
    }
    echo "ok\n";

    echo "return value identity for input params known first... ";
    flush();
    {
        $d1 = new Test\D1;
        $d1->sb = "D1.sb";
        $d1->sd1 = "D1.sd1";
        $d3 = new Test\D3;
        $d3->pb = $d1;
        $d3->sb = "D3.sb";
        $d3->sd3 = "D3.sd3";
        $d3->pd3 = $d1;
        $d1->pb = $d3;
        $d1->pd1 = $d3;

        $b1 = $test->returnTest3($d1, $d3);

        test($b1 != null);
        test($b1->sb == "D1.sb");
        test(get_class($b1) == "Test\D1");
        $p1 = $b1;
        test($p1 != null);
        test($p1->sd1 == "D1.sd1");
        test($p1->pd1 === $b1->pb); // Object identity comparison

        $b2 = $b1->pb;
        test($b2 != null);
        test($b2->sb == "D3.sb");
        test($b2->pb === $b1); // Object identity comparison

        $p3 = $b2;
        if($test->ice_getEncodingVersion() == $Ice_Encoding_1_0)
        {
            test(!($p3 instanceof Test\D3));
        }
        else
        {
            test(($p3 instanceof Test\D3));
            test($p3->pd3 === $p1);
            test($p3->sd3 == "D3.sd3");
        }

        test($b1 !== $d1);
        test($b1 !== $d3);
        test($b2 !== $d1);
        test($b2 !== $d3);

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $d1->pb = null;
        $d1->pd1 = null;
        $b1->pb = null;
        $p1->pd1 = null;
    }
    echo "ok\n";

    echo "return value identity for input params unknown first... ";
    flush();
    {
        $d1 = new Test\D1;
        $d1->sb = "D1.sb";
        $d1->sd1 = "D1.sd1";
        $d3 = new Test\D3;
        $d3->pb = $d1;
        $d3->sb = "D3.sb";
        $d3->sd3 = "D3.sd3";
        $d3->pd3 = $d1;
        $d1->pb = $d3;
        $d1->pd1 = $d3;

        $b1 = $test->returnTest3($d3, $d1);

        test($b1 != null);
        test($b1->sb == "D3.sb");

        $b2 = $b1->pb;
        test($b2 != null);
        test($b2->sb == "D1.sb");
        test(get_class($b2) == "Test\D1");
        test($b2->pb === $b1); // Object identity comparison
        $p3 = $b2;
        test($p3 instanceof Test\D1);
        test($p3->sd1 == "D1.sd1");
        test($p3->pd1 === $b1); // Object identity comparison

        $p1 = $b1;
        if($test->ice_getEncodingVersion() == $Ice_Encoding_1_0)
        {
            test(!($p1 instanceof Test\D3));
        }
        else
        {
            test(($p1 instanceof Test\D3));
            test($p1->pd3 === $b2);
            test($p1->sd3 == "D3.sd3");
        }

        test($b1 !== $d1);
        test($b1 !== $d3);
        test($b2 !== $d1);
        test($b2 !== $d3);

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $d1->pb = null;
        $d1->pd1 = null;
        $b1->pb = null;
        if($p1 instanceof Test\D3)
        {
            $p1->pd3 = null;
        }
    }
    echo "ok\n";

    echo "return value identity for input params unknown first... ";
    flush();
    {
        $d1 = new Test\D1;
        $d1->sb = "D1.sb";
        $d1->sd1 = "D1.sd1";
        $d3 = new Test\D3;
        $d3->pb = $d1;
        $d3->sb = "D3.sb";
        $d3->sd3 = "D3.sd3";
        $d3->pd3 = $d1;
        $d1->pb = $d3;
        $d1->pd1 = $d3;

        $b1 = $test->returnTest3($d3, $d1);

        test($b1 != null);
        test($b1->sb == "D3.sb");

        $b2 = $b1->pb;
        test($b2 != null);
        test($b2->sb == "D1.sb");
        test(get_class($b2) == "Test\D1");
        test($b2->pb === $b1); // Object identity comparison

        $p3 = $b2;
        test($p3 instanceof Test\D1);
        test($p3->sd1 == "D1.sd1");
        test($p3->pd1 === $b1); // Object identity comparison

        $p1 = $b1;
        if($test->ice_getEncodingVersion() == $Ice_Encoding_1_0)
        {
            test(!($p1 instanceof Test\D3));
        }
        else
        {
            test(($p1 instanceof Test\D3));
            test($p1->pd3 === $b2);
            test($p1->sd3 == "D3.sd3");
        }

        test($b1 !== $d1);
        test($b1 !== $d3);
        test($b2 !== $d1);
        test($b2 !== $d3);

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $d3->pb = null;
        $d3->pd3 = null;
        $d3->pb = null;
        $d3->pd3 = null;
        $b1->pb = null;
        $b2->pb = null;
    }
    echo "ok\n";

    echo "remainder unmarshaling (3 instances)... ";
    flush();
    {
        $ret = $test->paramTest3($p1, $p2);

        test($p1 != null);
        test($p1->sb == "D2.sb (p1 1)");
        test($p1->pb == null);
        test(get_class($p1) == "Test\B");

        test($p2 != null);
        test($p2->sb == "D2.sb (p2 1)");
        test($p2->pb == null);
        test(get_class($p2) == "Test\B");

        test($ret != null);
        test($ret->sb == "D1.sb (p2 2)");
        test($ret->pb === null);
        test(get_class($ret) == "Test\D1");
    }
    echo "ok\n";

    echo "remainder unmarshaling (4 instances)... ";
    flush();
    {
        $ret = $test->paramTest4($b);

        test($b != null);
        test($b->sb == "D4.sb (1)");
        test($b->pb == null);
        test(get_class($b) == "Test\B");

        test($ret != null);
        test($ret->sb == "B.sb (2)");
        test($ret->pb === null);
        test(get_class($ret) == "Test\B");
    }
    echo "ok\n";

    echo "parameter pointer slicing with first instance marshaled in unknown derived as base... ";
    flush();
    {
        $b1 = new Test\B;
        $b1->sb = "B.sb(1)";
        $b1->pb = $b1;

        $d3 = new Test\D3;
        $d3->sb = "D3.sb";
        $d3->pb = $d3;
        $d3->sd3 = "D3.sd3";
        $d3->pd3 = $b1;

        $b2 = new Test\B;
        $b2->sb = "B.sb(2)";
        $b2->pb = $b1;

        $r = $test->returnTest3($d3, $b2);

        test($r != null);
        test($r->sb == "D3.sb");
        test($r->pb === $r);

        $p3 = $r;
        if($test->ice_getEncodingVersion() == $Ice_Encoding_1_0)
        {
            test(!($p3 instanceof Test\D3));
        }
        else
        {
            test(($p3 instanceof Test\D3));

            test($p3->sb == "D3.sb");
            test($p3->pb === $r);
            test($p3->sd3 == "D3.sd3");

            test($p3->pd3->ice_id() == "::Test::B");
            test($p3->pd3->sb == "B.sb(1)");
            test($p3->pd3->pb === $p3->pd3);
        }

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $b1->pb = null;
        $d3->pb = null;
        $d3->pd3 = null;
        $r->pb = null;
    }
    echo "ok\n";

    echo "parameter pointer slicing with first instance marshaled in unknown derived as derived... ";
    flush();
    {
        $d11 = new Test\D1;
        $d11->sb = "D1.sb(1)";
        $d11->pb = $d11;
        $d11->pd1 = null;
        $d11->sd1 = "D1.sd1(1)";

        $d3 = new Test\D3;
        $d3->sb = "D3.sb";
        $d3->pb = $d3;
        $d3->sd3 = "D3.sd3";
        $d3->pd3 = $d11;

        $d12 = new Test\D1;
        $d12->sb = "D1.sb(2)";
        $d12->pb = $d12;
        $d12->sd1 = "D1.sd1(2)";
        $d12->pd1 = $d11;

        $r = $test->returnTest3($d3, $d12);
        test($r != null);
        test($r->sb == "D3.sb");
        test($r->pb === $r);

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $d11->pb = null;
        $d3->pb = null;
        $d3->pd3 = null;
        $r->pb = null;
        $d12->pb = null;
    }
    echo "ok\n";

    echo "sequence slicing... ";
    flush();
    {
        $ss = null;
        {
            $ss1b = new Test\B;
            $ss1b->sb = "B.sb";
            $ss1b->pb = $ss1b;

            $ss1d1 = new Test\D1;
            $ss1d1->sb = "D1.sb";
            $ss1d1->sd1 = "D1.sd1";
            $ss1d1->pb = $ss1b;

            $ss1d3 = new Test\D3;
            $ss1d3->sb = "D3.sb";
            $ss1d3->sd3 = "D3.sd3";
            $ss1d3->pb = $ss1b;
            $ss1d3->pd3 = null;

            $ss2b = new Test\B;
            $ss2b->sb = "B.sb";
            $ss2b->pb = $ss1b;

            $ss2d1 = new Test\D1;
            $ss2d1->sb = "D1.sb";
            $ss2d1->sd1 = "D1.sd1";
            $ss2d1->pb = $ss2b;

            $ss2d3 = new Test\D3;
            $ss2d3->sb = "D3.sb";
            $ss2d3->sd3 = "D3.sd3";
            $ss2d3->pb = $ss2b;

            $ss1d1->pd1 = $ss2b;
            $ss1d3->pd3 = $ss2d1;

            $ss2d1->pd1 = $ss1d3;
            $ss2d3->pd3 = $ss1d1;

            $ss1 = new Test\SS1;
            $ss1->s = array($ss1b, $ss1d1, $ss1d3);

            $ss2 = new Test\SS2;
            $ss2->s = array($ss2b, $ss2d1, $ss2d3);

            $ss = $test->sequenceTest($ss1, $ss2);

            // Break cyclic dependencies - helps in detecting leaks.
            $ss1b->pb = null;
            $ss1d1->pd1 = null;
            $ss1d3->pd3 = null;
            $ss2d1->pd1 = null;
            $ss2d3->pd3 = null;
        }

        test($ss->c1 != null);
        $ss1b = $ss->c1->s[0];
        $ss1d1 = $ss->c1->s[1];
        $ss1d3 = $ss->c1->s[2];

        test($ss->c2 != null);
        $ss2b = $ss->c2->s[0];
        $ss2d1 = $ss->c2->s[1];
        $ss2d3 = $ss->c2->s[2];

        test($ss1b->pb === $ss1b);
        test($ss1d1->pb === $ss1b);
        test($ss1d3->pb === $ss1b);

        test($ss2b->pb === $ss1b);
        test($ss2d1->pb === $ss2b);
        test($ss2d3->pb === $ss2b);

        test(get_class($ss1b) == "Test\B");
        test(get_class($ss1d1) == "Test\D1");

        test(get_class($ss2b) == "Test\B");
        test(get_class($ss2d1) == "Test\D1");

        if($test->ice_getEncodingVersion() == $Ice_Encoding_1_0)
        {
            test(get_class($ss1d3) == "Test\B");
            test(get_class($ss2d3) == "Test\B");
        }
        else
        {
            test(get_class($ss1d3) == "Test\D3");
            test(get_class($ss2d3) == "Test\D3");
        }

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $ss1b->pb = null;
    }
    echo "ok\n";

    echo "dictionary slicing... ";
    flush();
    {
        $bin = array();
        for($i = 0; $i < 10; $i++)
        {
            $d1 = new Test\D1;
            $d1->sb = sprintf("D1.%d", $i);
            $d1->pb = $d1;
            $d1->sd1 = $d1->sb;
            $d1->pd1 = null;
            $bin[$i] = $d1;
        }

        $r = $test->dictionaryTest($bin, $bout);

        test(count($bout) == 10);
        for($i = 0; $i < 10; $i++)
        {
            $b = $bout[$i * 10];
            $s = sprintf("D1.%d", $i);
            test($b != null);
            test($b->sb == $s);
            test($b->pb != null);
            test($b->pb !== $b);
            test($b->pb->sb == $s);
            test($b->pb->pb === $b->pb);
        }

        test(count($r) == 10);
        for($i = 0; $i < 10; $i++)
        {
            $b = $r[$i * 20];
            $s = sprintf("D1.%d", $i * 20);
            test($b != null);
            test($b->sb == $s);
            if($i == 0)
            {
                test($b->pb == null);
            }
            else
            {
                test($b->pb === $r[($i - 1) * 20]);
            }
            test($b instanceof Test\D1);
            $d1 = $b;
            test($d1->sd1 == $s);
            test($d1->pd1 === $d1);
        }

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        for($i = 0; $i < 10; $i++)
        {
            $bin[$i]->pb = null;
            $bout[$i * 10]->pb->pb = null;
            $r[$i * 20]->pb = null;
            $r[$i * 20]->pd1 = null;
        }
    }
    echo "ok\n";

    echo "base exception thrown as base exception... ";
    flush();
    {
        try
        {
            $test->throwBaseAsBase();
            test(false);
        }
        catch(Test\BaseException $e)
        {
            test(get_class($e) == "Test\\BaseException");
            test($e->sbe == "sbe");
            test($e->pb != null);
            test($e->pb->sb == "sb");
            test($e->pb->pb === $e->pb);

            // Break cyclic dependencies - helps in detecting leaks.
            $e->pb->pb = null;
        }
    }
    echo "ok\n";

    echo "derived exception thrown as base exception... ";
    flush();
    {
        try
        {
            $test->throwDerivedAsBase();
            test(false);
        }
        catch(Test\DerivedException $e)
        {
            test(get_class($e) == "Test\DerivedException");
            test($e->sbe == "sbe");
            test($e->pb != null);
            test($e->pb->sb == "sb1");
            test($e->pb->pb === $e->pb);
            test($e->sde == "sde1");
            test($e->pd1 != null);
            test($e->pd1->sb == "sb2");
            test($e->pd1->pb === $e->pd1);
            test($e->pd1->sd1 == "sd2");
            test($e->pd1->pd1 === $e->pd1);

            //
            // Break cyclic dependencies - helps in detecting leaks.
            //
            $e->pb->pb = null;
            $e->pd1->pb = null;
            $e->pd1->pd1 = null;
        }
    }
    echo "ok\n";

    echo "derived exception thrown as derived exception... ";
    flush();
    {
        try
        {
            $test->throwDerivedAsDerived();
            test(false);
        }
        catch(Test\DerivedException $e)
        {
            test(get_class($e) == "Test\DerivedException");
            test($e->sbe == "sbe");
            test($e->pb != null);
            test($e->pb->sb == "sb1");
            test($e->pb->pb === $e->pb);
            test($e->sde == "sde1");
            test($e->pd1 != null);
            test($e->pd1->sb == "sb2");
            test($e->pd1->pb === $e->pd1);
            test($e->pd1->sd1 == "sd2");
            test($e->pd1->pd1 === $e->pd1);

            //
            // Break cyclic dependencies - helps in detecting leaks.
            //
            $e->pb->pb = null;
            $e->pd1->pb = null;
            $e->pd1->pd1 = null;
        }
    }
    echo "ok\n";

    echo "unknown derived exception thrown as base exception... ";
    flush();
    {
        try
        {
            $test->throwUnknownDerivedAsBase();
            test(false);
        }
        catch(Test\BaseException $e)
        {
            test(get_class($e) == "Test\\BaseException");
            test($e->sbe == "sbe");
            test($e->pb != null);
            test($e->pb->sb == "sb d2");
            test($e->pb->pb === $e->pb);

            // Break cyclic dependencies - helps in detecting leaks.
            $e->pb->pb = null;
        }
    }
    echo "ok\n";

    echo "forward-declared class... ";
    flush();
    {
        try
        {
            $f = null;
            $test->useForward($f);
            test($f != null);
        }
        catch(Exception $e)
        {
            test(false);
        }
    }
    echo "ok\n";

    echo "preserved classes... ";
    flush();
    {
        //
        // Server knows the most-derived class PDerived.
        //
        $pd = new Test\PDerived;
        $pd->pi = 3;
        $pd->ps = "preserved";
        $pd->pb = $pd;

        $r = $test->exchangePBase($pd);
        test(get_class($r) == "Test\\PDerived");
        test($r->pi == 3);
        test($r->ps == "preserved");
        test($r->pb === $r); // Object identity comparison

        //
        // Server only knows the base (non-preserved) type, so the object is sliced.
        //
        $pu = new Test\PCUnknown;
        $pu->pi = 3;
        $pu->pu = "preserved";

        $r = $test->exchangePBase($pu);
        test($r->pi == 3);

        $p2 = $r;
        if($test->ice_getEncodingVersion() == $Ice_Encoding_1_0)
        {
            test(get_class($p2) != "Test\\PCUnknown");
        }
        else
        {
            test(get_class($p2) == "Test\\PCUnknown");
            test($p2->pu == "preserved");
        }

        //
        // Server only knows the intermediate type Preserved. The object will be sliced to
        // Preserved for the 1.0 encoding; otherwise it should be returned intact.
        //
        $pcd = new Test\PCDerived;
        $pcd->pi = 3;
        $pcd->pbs = array($pcd);

        $r = $test->exchangePBase($pcd);
        if($test->ice_getEncodingVersion() == $Ice_Encoding_1_0)
        {
            test(get_class($r) != "Test\\PCDerived");
            test($r->pi == 3);
        }
        else
        {
            test(get_class($r) == "Test\\PCDerived");
            test($r->pi == 3);
            test($r->pbs[0] === $r); // Object identity comparison
        }

        //
        // Server only knows the intermediate type CompactPDerived. The object will be sliced to
        // CompactPDerived for the 1.0 encoding; otherwise it should be returned intact.
        //
        $pcd = new Test\CompactPCDerived;
        $pcd->pi = 3;
        $pcd->pbs = array($pcd);

        $r = $test->exchangePBase($pcd);
        if($test->ice_getEncodingVersion() == $Ice_Encoding_1_0)
        {
            test(get_class($r) != "Test\\CompactPCDerived");
            test($r->pi == 3);
        }
        else
        {
            test(get_class($r) == "Test\\CompactPCDerived");
            test($r->pi == 3);
            test($r->pbs[0] === $r); // Object identity comparison
        }

        //
        // Send an object that will have multiple preserved slices in the server.
        // The object will be sliced to Preserved for the 1.0 encoding.
        //
        $pcd = new Test\PCDerived3;
        $pcd->pi = 3;
        //
        // Sending more than 254 objects exercises the encoding for object ids.
        //
        $pcd->pbs = array();
        for($i = 0; $i < 300; ++$i)
        {
            $p2 = new Test\PCDerived2;
            $p2->pi = $i;
            $p2->pbs = array(null); // Nil reference. This slice should not have an indirection table.
            $p2->pcd2 = $i;
            array_push($pcd->pbs, $p2);
        }
        $pcd->pcd2 = $pcd->pi;
        $pcd->pcd3 = $pcd->pbs[10];

        $r = $test->exchangePBase($pcd);
        if($test->ice_getEncodingVersion() == $Ice_Encoding_1_0)
        {
            test(get_class($r) != "Test\\PCDerived3");
            test(get_class($r) == "Test\\PDerived");
            test($r->pi == 3);
        }
        else
        {
            test(get_class($r) == "Test\\PCDerived3");
            test($r->pi == 3);
            for($i = 0; $i < 300; ++$i)
            {
                $p2 = $r->pbs[$i];
                test(get_class($p2) == "Test\\PCDerived2");
                test($p2->pi == $i);
                test(count($p2->pbs) == 1);
                test($p2->pbs[0] == null);
                test($p2->pcd2 == $i);
            }
            test($r->pcd2 == $r->pi);
            test($r->pcd3 === $r->pbs[10]); // Object identity comparison
        }

        //
        // Obtain an object with preserved slices and send it back to the server.
        // The preserved slices should be excluded for the 1.0 encoding, otherwise
        // they should be included.
        //
        $p = $test->PBSUnknownAsPreserved();
        $test->checkPBSUnknown($p);
        if($test->ice_getEncodingVersion() != $Ice_Encoding_1_0)
        {
            $slicedData = $p->ice_getSlicedData();
            test(count($slicedData->slices) == 1);
            test($slicedData->slices[0]->typeId == "::Test::PSUnknown");
            $test->ice_encodingVersion($Ice_Encoding_1_0)->checkPBSUnknown($p);
        }
        else
        {
            test(!$p->ice_getSlicedData());
        }

        //
        // Relay a graph through the server. This test uses a preserved class
        // with a class member.
        //
        $c = new Test\PNode;
        $c->next = new Test\PNode;
        $c->next->next = new Test\PNode;
        $c->next->next->next = $c;    // Create a cyclic graph.

        $n = $test->exchangePNode($c);
        test($n->next != null);
        test($n->next !== $n->next->next); // Object identity comparison
        test($n->next->next !== $n->next->next->next); // Object identity comparison
        test($n->next->next->next === $n); // Object identity comparison
        $n = null;      // Release reference.

        //
        // Obtain a preserved object from the server where the most-derived
        // type is unknown. The preserved slice refers to a graph of PNode
        // objects.
        //
        $p = $test->PBSUnknownAsPreservedWithGraph();
        test($p != null);
        $test->checkPBSUnknownWithGraph($p);
        $p = null;      // Release reference.

        //
        // Obtain a preserved object from the server where the most-derived
        // type is unknown. A data member in the preserved slice refers to the
        // outer object, so the chain of references looks like this:
        //
        // outer->slicedData->outer
        //
        $p = $test->PBSUnknown2AsPreservedWithGraph();
        test($p != null);
        if($test->ice_getEncodingVersion() != $Ice_Encoding_1_0)
        {
            test($p->_ice_slicedData != null);
        }
        $test->checkPBSUnknown2WithGraph($p);
        $p->_ice_slicedData = null;     // Break the cycle.
        $p = null;                      // Release reference.
    }
    echo "ok\n";

    return $test;
}

class Client extends TestHelper
{
    function run($args)
    {
        try
        {
            $communicator = $this->initialize($args);
            $proxy = allTests($this);
            $proxy->shutdown();
            $communicator->destroy();
        }
        catch(Exception $ex)
        {
            $communicator->destroy();
            throw $ex;
       }
    }
}
