<?
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

function allTests()
{
    global $ICE;

    $obj = $ICE->stringToProxy("Test:default -p 12010");
    $test = $obj->ice_checkedCast("::Test::TestIntf");

    echo "testing base as Object... ";
    flush();
    {
        $o = $test->SBaseAsObject();
        test($o != null);
        test(get_class($o) == "Test_SBase");
        test($o->sb == "SBase.sb");
    }
    echo "ok\n";

    echo "testing base as base... ";
    flush();
    {
        $o = $test->SBaseAsSBase();
        test($o->sb == "SBase.sb");
    }
    echo "ok\n";

    echo "testing base with known derived as base... ";
    flush();
    {
        $sb = $test->SBSKnownDerivedAsSBase();
        test($sb->sb == "SBSKnownDerived.sb");
        test(get_class($sb) == "Test_SBSKnownDerived");
        test($sb->sbskd == "SBSKnownDerived.sbskd");
    }
    echo "ok\n";

    echo "testing base with known derived as known derived... ";
    flush();
    {
        $sbksd = $test->SBSKnownDerivedAsSBSKnownDerived();
        test($sbksd->sbskd == "SBSKnownDerived.sbskd");
    }
    echo "ok\n";

    echo "testing base with unknown derived as base... ";
    flush();
    {
        $sb = $test->SBSUnknownDerivedAsSBase();
        test($sb->sb == "SBSUnknownDerived.sb");
    }
    echo "ok\n";

    echo "testing unknown with Object as Object... ";
    flush();
    {
        try
        {
            $o = $test->SUnknownAsObject();
            test(false);
        }
        catch(Ice_NoObjectFactoryException $e)
        {
            test(get_class($e) == "Ice_NoObjectFactoryException");
        }
    }
    echo "ok\n";

    echo "testing one-element cycle... ";
    flush();
    {
        $b = $test->oneElementCycle();
        test($b != null);
        test(get_class($b) == "Test_B");
        test($b->sb == "B1.sb");
        test($b->pb === $b); // Object identity comparison

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $b->pb = null;
    }
    echo "ok\n";

    echo "testing two-element cycle... ";
    flush();
    {
        $b1 = $test->twoElementCycle();
        test($b1 != null);
        test(get_class($b1) == "Test_B");
        test($b1->sb == "B1.sb");

        $b2 = $b1->pb;
        test($b2 != null);
        test(get_class($b2) == "Test_B");
        test($b2->sb == "B2.sb");
        test($b2->pb === $b1); // Object identity comparison

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $b1->pb = null;
    }
    echo "ok\n";

    echo "testing known derived pointer slicing as base... ";
    flush();
    {
        $b1 = $test->D1AsB();
        test($b1 != null);
        test(get_class($b1) == "Test_D1");
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
        test(get_class($b2) == "Test_B");

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $b2->pb = null;
    }
    echo "ok\n";

    echo "testing known derived pointer slicing as derived... ";
    flush();
    {
        $d1 = $test->D1AsD1();
        test($d1 != null);
        test(get_class($d1) == "Test_D1");
        test($d1->sb == "D1.sb");
        test($d1->pb != null);
        test($d1->pb !== $d1); // Object identity comparison

        $b2 = $d1->pb;
        test($b2 != null);
        test(get_class($b2) == "Test_B");
        test($b2->sb == "D2.sb");
        test($b2->pb === $d1); // Object identity comparison

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $b2->pb = null;
    }
    echo "ok\n";

    echo "testing unknown derived pointer slicing as base... ";
    flush();
    {
        $b2 = $test->D2AsB();
        test($b2 != null);
        test(get_class($b2) == "Test_B");
        test($b2->sb == "D2.sb");
        test($b2->pb != null);
        test($b2->pb !== $b2); // Object identity comparison

        $b1 = $b2->pb;
        test($b1 != null);
        test(get_class($b1) == "Test_D1");
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

    echo "testing parameter pointer slicing with known first... ";
    flush();
    {
        $test->paramTest1($b1, $b2);

        test($b1 != null);
        test(get_class($b1) == "Test_D1");
        test($b1->sb == "D1.sb");
        test($b1->pb === $b2); // Object identity comparison
        $d1 = $b1;
        test($d1 != null);
        test($d1->sd1 == "D1.sd1");
        test($d1->pd1 === $b2); // Object identity comparison

        test($b2 != null);
        test(get_class($b2) == "Test_B"); // No factory, must be sliced
        test($b2->sb == "D2.sb");
        test($b2->pb === $b1); // Object identity comparison

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $b2->pb = null;
    }
    echo "ok\n";

    echo "testing parameter pointer slicing with unknown first... ";
    flush();
    {
        $test->paramTest2($b2, $b1);

        test($b1 != null);
        test(get_class($b1) == "Test_D1");
        test($b1->sb == "D1.sb");
        test($b1->pb === $b2); // Object identity comparison
        $d1 = $b1;
        test($d1 != null);
        test($d1->sd1 == "D1.sd1");
        test($d1->pd1 === $b2); // Object identity comparison

        test($b2 != null);
        test(get_class($b2) == "Test_B"); // No factory, must be sliced
        test($b2->sb == "D2.sb");
        test($b2->pb === $b1); // Object identity comparison

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $b2->pb = null;
    }
    echo "ok\n";

    echo "testing return value identity with known first... ";
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

    echo "testing return value identity with unknown first... ";
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

    echo "testing return value identity for input params known first... ";
    flush();
    {
        $d1 = new Test_D1;
        $d1->sb = "D1.sb";
        $d1->sd1 = "D1.sd1";
        $d3 = new Test_D3;
        $d3->pb = $d1;
        $d3->sb = "D3.sb";
        $d3->sd3 = "D3.sd3";
        $d3->pd3 = $d1;
        $d1->pb = $d3;
        $d1->pd1 = $d3;

        $b1 = $test->returnTest3($d1, $d3);

        test($b1 != null);
        test($b1->sb == "D1.sb");
        test(get_class($b1) == "Test_D1");
        $p1 = $b1;
        test($p1 != null);
        test($p1->sd1 == "D1.sd1");
        test($p1->pd1 === $b1->pb); // Object identity comparison

        $b2 = $b1->pb;
        test($b2 != null);
        test($b2->sb == "D3.sb");
        test(get_class($b2) == "Test_B"); // Sliced by server
        test($b2->pb === $b1); // Object identity comparison
        test(!($b2 instanceof Test_D3));

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

    echo "testing return value identity for input params unknown first... ";
    flush();
    {
        $d1 = new Test_D1;
        $d1->sb = "D1.sb";
        $d1->sd1 = "D1.sd1";
        $d3 = new Test_D3;
        $d3->pb = $d1;
        $d3->sb = "D3.sb";
        $d3->sd3 = "D3.sd3";
        $d3->pd3 = $d1;
        $d1->pb = $d3;
        $d1->pd1 = $d3;

        $b1 = $test->returnTest3($d3, $d1);

        test($b1 != null);
        test($b1->sb == "D3.sb");
        test(get_class($b1) == "Test_B"); // Sliced by server
        test(!($b1 instanceof Test_D3));

        $b2 = $b1->pb;
        test($b2 != null);
        test($b2->sb == "D1.sb");
        test(get_class($b2) == "Test_D1");
        test($b2->pb === $b1); // Object identity comparison
        $p3 = $b2;
        test($p3 instanceof Test_D1);
        test($p3->sd1 == "D1.sd1");
        test($p3->pd1 === $b1); // Object identity comparison

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

    echo "testing return value identity for input params unknown first... ";
    flush();
    {
        $d1 = new Test_D1;
        $d1->sb = "D1.sb";
        $d1->sd1 = "D1.sd1";
        $d3 = new Test_D3;
        $d3->pb = $d1;
        $d3->sb = "D3.sb";
        $d3->sd3 = "D3.sd3";
        $d3->pd3 = $d1;
        $d1->pb = $d3;
        $d1->pd1 = $d3;

        $b1 = $test->returnTest3($d3, $d1);

        test($b1 != null);
        test($b1->sb == "D3.sb");
        test(get_class($b1) == "Test_B"); // Sliced by server
        test(!($b1 instanceof Test_D3));

        $b2 = $b1->pb;
        test($b2 != null);
        test($b2->sb == "D1.sb");
        test(get_class($b2) == "Test_D1");
        test($b2->pb === $b1); // Object identity comparison
        $p3 = $b2;
        test($p3 instanceof Test_D1);
        test($p3->sd1 == "D1.sd1");
        test($p3->pd1 === $b1); // Object identity comparison

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

    echo "testing remainder unmarshaling (3 instances)... ";
    flush();
    {
        $ret = $test->paramTest3($p1, $p2);

        test($p1 != null);
        test($p1->sb == "D2.sb (p1 1)");
        test($p1->pb == null);
        test(get_class($p1) == "Test_B");

        test($p2 != null);
        test($p2->sb == "D2.sb (p2 1)");
        test($p2->pb == null);
        test(get_class($p2) == "Test_B");

        test($ret != null);
        test($ret->sb == "D1.sb (p2 2)");
        test($ret->pb === null);
        test(get_class($ret) == "Test_D1");
    }
    echo "ok\n";

    echo "testing remainder unmarshaling (4 instances)... ";
    flush();
    {
        $ret = $test->paramTest4($b);

        test($b != null);
        test($b->sb == "D4.sb (1)");
        test($b->pb == null);
        test(get_class($b) == "Test_B");

        test($ret != null);
        test($ret->sb == "B.sb (2)");
        test($ret->pb === null);
        test(get_class($ret) == "Test_B");
    }
    echo "ok\n";

    echo "testing parameter pointer slicing with first instance marshaled in unknown derived as base... ";
    flush();
    {
        $b1 = new Test_B;
        $b1->sb = "B.sb(1)";
        $b1->pb = $b1;

        $d3 = new Test_D3;
        $d3->sb = "D3.sb";
        $d3->pb = $d3;
        $d3->sd3 = "D3.sd3";
        $d3->pd3 = $b1;

        $b2 = new Test_B;
        $b2->sb = "B.sb(2)";
        $b2->pb = $b1;

        $r = $test->returnTest3($d3, $b2);

        test($r != null);
        test(get_class($r) == "Test_B");
        test($r->sb == "D3.sb");
        test($r->pb === $r);

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $b1->pb = null;
        $d3->pb = null;
        $d3->pd3 = null;
        $r->pb = null;
    }
    echo "ok\n";

    echo "testing parameter pointer slicing with first instance marshaled in unknown derived as derived... ";
    flush();
    {
        $d11 = new Test_D1;
        $d11->sb = "D1.sb(1)";
        $d11->pb = $d11;
        $d11->pd1 = null;
        $d11->sd1 = "D1.sd1(1)";

        $d3 = new Test_D3;
        $d3->sb = "D3.sb";
        $d3->pb = $d3;
        $d3->sd3 = "D3.sd3";
        $d3->pd1 = null;
        $d3->pd3 = $d11;

        $d12 = new Test_D1;
        $d12->sb = "D1.sb(2)";
        $d12->pb = $d12;
        $d12->sd1 = "D1.sd1(2)";
        $d12->pd1 = $d11;

        $r = $test->returnTest3($d3, $d12);
        test($r != null);
        test(get_class($r) == "Test_B");
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

    echo "testing sequence slicing... ";
    flush();
    {
        $ss = null;
        {
            $ss1b = new Test_B;
            $ss1b->sb = "B.sb";
            $ss1b->pb = $ss1b;

            $ss1d1 = new Test_D1;
            $ss1d1->sb = "D1.sb";
            $ss1d1->sd1 = "D1.sd1";
            $ss1d1->pb = $ss1b;

            $ss1d3 = new Test_D3;
            $ss1d3->sb = "D3.sb";
            $ss1d3->sd3 = "D3.sd3";
            $ss1d3->pb = $ss1b;
            $ss1d3->pd3 = null;

            $ss2b = new Test_B;
            $ss2b->sb = "B.sb";
            $ss2b->pb = $ss1b;

            $ss2d1 = new Test_D1;
            $ss2d1->sb = "D1.sb";
            $ss2d1->sd1 = "D1.sd1";
            $ss2d1->pb = $ss2b;

            $ss2d3 = new Test_D3;
            $ss2d3->sb = "D3.sb";
            $ss2d3->sd3 = "D3.sd3";
            $ss2d3->pb = $ss2b;
            $ss2d3->pd1 = null;

            $ss1d1->pd1 = $ss2b;
            $ss1d3->pd3 = $ss2d1;

            $ss2d1->pd1 = $ss1d3;
            $ss2d3->pd3 = $ss1d1;

            $ss1 = new Test_SS1;
            $ss1->s = array($ss1b, $ss1d1, $ss1d3);

            $ss2 = new Test_SS2;
            $ss2->s = array($ss2b, $ss2d1, $ss2d3);

            $ss = $test->sequenceTest($ss1, $ss2);

            //
            // Break cyclic dependencies - helps in detecting leaks.
            //
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

        test(get_class($ss1b) == "Test_B");
        test(get_class($ss1d1) == "Test_D1");
        test(get_class($ss1d3) == "Test_B");

        test(get_class($ss2b) == "Test_B");
        test(get_class($ss2d1) == "Test_D1");
        test(get_class($ss2d3) == "Test_B");

        //
        // Break cyclic dependencies - helps in detecting leaks.
        //
        $ss1b->pb = null;
    }
    echo "ok\n";

    echo "testing dictionary slicing... ";
    flush();
    {
        $bin = array();
        for($i = 0; $i < 10; $i++)
        {
            $d1 = new Test_D1;
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
            test($b instanceof Test_D1);
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

    echo "testing base exception thrown as base exception... ";
    flush();
    {
        try
        {
            $test->throwBaseAsBase();
            test(false);
        }
        catch(Test_BaseException $e)
        {
            test(get_class($e) == "Test_BaseException");
            test($e->sbe == "sbe");
            test($e->pb != null);
            test($e->pb->sb == "sb");
            test($e->pb->pb === $e->pb);

            //
            // Break cyclic dependencies - helps in detecting leaks.
            //
            $e->pb->pb = null;
        }
    }
    echo "ok\n";

    echo "testing derived exception thrown as base exception... ";
    flush();
    {
        try
        {
            $test->throwDerivedAsBase();
            test(false);
        }
        catch(Test_DerivedException $e)
        {
            test(get_class($e) == "Test_DerivedException");
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

    echo "testing derived exception thrown as derived exception... ";
    flush();
    {
        try
        {
            $test->throwDerivedAsDerived();
            test(false);
        }
        catch(Test_DerivedException $e)
        {
            test(get_class($e) == "Test_DerivedException");
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

    echo "testing unknown derived exception thrown as base exception... ";
    flush();
    {
        try
        {
            $test->throwUnknownDerivedAsBase();
            test(false);
        }
        catch(Test_BaseException $e)
        {
            test(get_class($e) == "Test_BaseException");
            test($e->sbe == "sbe");
            test($e->pb != null);
            test($e->pb->sb == "sb d2");
            test($e->pb->pb === $e->pb);

            //
            // Break cyclic dependencies - helps in detecting leaks.
            //
            $e->pb->pb = null;
        }
    }
    echo "ok\n";

    return $test;
}

$test = allTests();
$test->shutdown();
exit();
?>
