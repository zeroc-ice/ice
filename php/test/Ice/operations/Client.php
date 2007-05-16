<?
// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

function twoways($communicator, $p)
{
    global $ICE;

    {
        $p->opVoid();
    }

    {
        $r = $p->opByte(0xff, 0x0f, $b);
        test($b == 0xf0);
        test($r == 0xff);
    }

    {
        $r = $p->opBool(true, false, $b);
        test($b);
        test(!$r);
    }

    {
        $r = $p->opShortIntLong(10, 11, 12, $s, $i, $l);
        test($s == 10);
        test($i == 11);
        test($l == 12);
        test($r == 12);

        $SHORT_MIN = -32768;
        $INT_MIN = -2147483647 - 1;
        $LONG_MIN = "-9223372036854775808"; // Must be a string
        $r = $p->opShortIntLong($SHORT_MIN, $INT_MIN, $LONG_MIN, $s, $i, $l);
        test($s == $SHORT_MIN);
        test($i == $INT_MIN);
        test($l == $LONG_MIN);
        test($r == $LONG_MIN);

        $SHORT_MAX = 32767;
        $INT_MAX = 2147483647;
        $LONG_MAX = "9223372036854775807"; // Must be a string
        $r = $p->opShortIntLong($SHORT_MAX, $INT_MAX, $LONG_MAX, $s, $i, $l);
        test($s == $SHORT_MAX);
        test($i == $INT_MAX);
        test($l == $LONG_MAX);
        test($r == $LONG_MAX);
    }

    {
        $r = $p->opFloatDouble(3.14, 1.1E10, $f, $d);
        test($f - 3.14 < 0.0001);
        test($d == 1.1E10);
        test($r == 1.1E10);
    }

    {
        $r = $p->opString("hello", "world", $s);
        test($s == "world hello");
        test($r == "hello world");
    }

    {
        $r = $p->opMyEnum(Test_MyEnum::enum2, $e);
        test($e == Test_MyEnum::enum2);
        test($r == Test_MyEnum::enum3);
    }

    {
        $r = $p->opMyClass($p, $c1, $c2);
        // TODO: Identity tests
        test($c1->ice_getIdentity() == $ICE->stringToIdentity("test"));
        test($c2->ice_getIdentity() == $ICE->stringToIdentity("noSuchIdentity"));
        test($r->ice_getIdentity() == $ICE->stringToIdentity("test"));
        $r->opVoid();
        $c1->opVoid();
        try
        {
            $c2->opVoid();
            test(false);
        }
        catch(Ice_LocalException $ex)
        {
        }

        $r = $p->opMyClass(null, $c1, $c2);
        test($c1 == null);
        test($c2 != null);
        $r->opVoid();
    }

    {
        $si1 = new Test_Structure;
        $si1->p = $p;
        $si1->e = Test_MyEnum::enum3;
        $si1->s = new Test_AnotherStruct;
        $si1->s->s = "abc";
        $si2 = new Test_Structure;
        $si2->p = null;
        $si2->e = Test_MyEnum::enum2;
        $si2->s = new Test_AnotherStruct;
        $si2->s->s = "def";

        $rso = $p->opStruct($si1, $si2, $so);
        test($rso->p == null);
        test($rso->e == Test_MyEnum::enum2);
        test($rso->s->s == "def");
        test($so->p == $p);
        test($so->e == Test_MyEnum::enum3);
        test($so->s->s == "a new string");
        $so->p->opVoid();
    }

    {
        $bsi1 = array(0x01, 0x11, 0x12, 0x22);
        $bsi2 = array(0xf1, 0xf2, 0xf3, 0xf4);
        $rso = $p->opByteS($bsi1, $bsi2, $bso);
        test(count($bso) == 4);
        test($bso[0] == 0x22);
        test($bso[1] == 0x12);
        test($bso[2] == 0x11);
        test($bso[3] == 0x01);
        test(count($rso) == 8);
        test($rso[0] == 0x01);
        test($rso[1] == 0x11);
        test($rso[2] == 0x12);
        test($rso[3] == 0x22);
        test($rso[4] == 0xf1);
        test($rso[5] == 0xf2);
        test($rso[6] == 0xf3);
        test($rso[7] == 0xf4);
    }

    {
        $bsi1 = array(true, true, false);
        $bsi2 = array(false);
        $rso = $p->opBoolS($bsi1, $bsi2, $bso);
        test(count($bso) == 4);
        test($bso[0]);
        test($bso[1]);
        test(!$bso[2]);
        test(!$bso[3]);
        test(count($rso) == 3);
        test(!$rso[0]);
        test($rso[1]);
        test($rso[2]);
    }

    {
        $ssi = array(1, 2, 3);
        $isi = array(5, 6, 7, 8);
        $lsi = array(10, 30, 20);
        $rso = $p->opShortIntLongS($ssi, $isi, $lsi, $sso, $iso, $lso);
        test(count($sso) == 3);
        test($sso[0] == 1);
        test($sso[1] == 2);
        test($sso[2] == 3);
        test(count($iso) == 4);
        test($iso[0] == 8);
        test($iso[1] == 7);
        test($iso[2] == 6);
        test($iso[3] == 5);
        test(count($lso) == 6);
        test($lso[0] == 10);
        test($lso[1] == 30);
        test($lso[2] == 20);
        test($lso[3] == 10);
        test($lso[4] == 30);
        test($lso[5] == 20);
        test(count($rso) == 3);
        test($rso[0] == 10);
        test($rso[1] == 30);
        test($rso[2] == 20);
    }

    {
        $fsi = array(3.14, 1.11);
        $dsi = array(1.1E10, 1.2E10, 1.3E10);
        $rso = $p->opFloatDoubleS($fsi, $dsi, $fso, $dso);
        test(count($fso) == 2);
        test($fso[0] - 3.14 < 0.0001);
        test($fso[1] - 1.11 < 0.0001);
        test(count($dso) == 3);
        test($dso[0] == 1.3E10);
        test($dso[1] == 1.2E10);
        test($dso[2] == 1.1E10);
        test(count($rso) == 5);
        test($rso[0] == 1.1E10);
        test($rso[1] == 1.2E10);
        test($rso[2] == 1.3E10);
        test($rso[3] - 3.14 < 0.0001);
        test($rso[4] - 1.11 < 0.0001);
    }

    {
        $ssi1 = array("abc", "de", "fghi");
        $ssi2 = array("xyz");
        $rso = $p->opStringS($ssi1, $ssi2, $sso);
        test(count($sso) == 4);
        test($sso[0] == "abc");
        test($sso[1] == "de");
        test($sso[2] == "fghi");
        test($sso[3] == "xyz");
        test(count($rso) == 3);
        test($rso[0] == "fghi");
        test($rso[1] == "de");
        test($rso[2] == "abc");
    }

    {
        $bsi1 = array(array(0x01, 0x11, 0x12), array(0xff));
        $bsi2 = array(array(0x0e), array(0xf2, 0xf1));
        $rso = $p->opByteSS($bsi1, $bsi2, $bso);
        test(count($bso) == 2);
        test(count($bso[0]) == 1);
        test($bso[0][0] == 0xff);
        test(count($bso[1]) == 3);
        test($bso[1][0] == 0x01);
        test($bso[1][1] == 0x11);
        test($bso[1][2] == 0x12);
        test(count($rso) == 4);
        test(count($rso[0]) == 3);
        test($rso[0][0] == 0x01);
        test($rso[0][1] == 0x11);
        test($rso[0][2] == 0x12);
        test($bso[0][0] == 0xff);
        test(count($rso[1]) == 1);
        test($rso[1][0] == 0xff);
        test(count($rso[2]) == 1);
        test($rso[2][0] == 0x0e);
        test(count($rso[3]) == 2);
        test($rso[3][0] == 0xf2);
        test($rso[3][1] == 0xf1);
    }

    {
        $fsi = array(array(3.14), array(1.11), array());
        $dsi = array(array(1.1E10, 1.2E10, 1.3E10));
        $rso = $p->opFloatDoubleSS($fsi, $dsi, $fso, $dso);
        test(count($fso) == 3);
        test(count($fso[0]) == 1);
        test($fso[0][0] - 3.14 < 0.0001);
        test(count($fso[1]) == 1);
        test($fso[1][0] - 1.11 < 0.0001);
        test(count($fso[2]) == 0);
        test(count($dso) == 1);
        test(count($dso[0]) == 3);
        test($dso[0][0] == 1.1E10);
        test($dso[0][1] == 1.2E10);
        test($dso[0][2] == 1.3E10);
        test(count($rso) == 2);
        test(count($rso[0]) == 3);
        test($rso[0][0] == 1.1E10);
        test($rso[0][1] == 1.2E10);
        test($rso[0][2] == 1.3E10);
        test(count($rso[1]) == 3);
        test($rso[1][0] == 1.1E10);
        test($rso[1][1] == 1.2E10);
        test($rso[1][2] == 1.3E10);
    }

    {
        $ssi1 = array(array("abc"), array("de", "fghi"));
        $ssi2 = array(array(), array(), array("xyz"));
        $rso = $p->opStringSS($ssi1, $ssi2, $sso);
        test(count($sso) == 5);
        test(count($sso[0]) == 1);
        test($sso[0][0] == "abc");
        test(count($sso[1]) == 2);
        test($sso[1][0] == "de");
        test($sso[1][1] == "fghi");
        test(count($sso[2]) == 0);
        test(count($sso[3]) == 0);
        test(count($sso[4]) == 1);
        test($sso[4][0] == "xyz");
        test(count($rso) == 3);
        test(count($rso[0]) == 1);
        test($rso[0][0] == "xyz");
        test(count($rso[1]) == 0);
        test(count($rso[2]) == 0);
    }

    {
        $di1 = array(10 => true, 100 => false);
        $di2 = array(10 => true, 11 => false, 101 => true);
        $ro = $p->opByteBoolD($di1, $di2, $_do);
        test($_do == $di1);
        test(count($ro) == 4);
        test($ro[10]);
        test(!$ro[11]);
        test(!$ro[100]);
        test($ro[101]);
    }

    {
        $di1 = array(110 => -1, 1100 => 123123);
        $di2 = array(110 => -1, 111 => -100, 1101 => 0);
        $ro = $p->opShortIntD($di1, $di2, $_do);
        test($_do == $di1);
        test(count($ro) == 4);
        test($ro[110] == -1);
        test($ro[111] == -100);
        test($ro[1100] == 123123);
        test($ro[1101] == 0);
    }

    {
        $di1 = array(999999110 => -1.1, 999999111 => 123123.2);
        $di2 = array(999999110 => -1.1, 999999120 => -100.4, 999999130 => 0.5);
        $ro = $p->opLongFloatD($di1, $di2, $_do);
        test(count($_do) == 2);
        test($_do[999999110] - $di1[999999110] < 0.0001);
        test($_do[999999111] - $di1[999999111] < 0.01);
        test(count($ro) == 4);
        test($ro[999999110] - -1.1 < 0.0001);
        test($ro[999999120] - -100.4 < 0.0001);
        test($ro[999999111] - 123123.2 < 0.01);
        test($ro[999999130] - 0.5 < 0.0001);
    }

    {
        $di1 = array("foo" => "abc -1.1", "bar" => "abc 123123.2");
        $di2 = array("foo" => "abc -1.1", "FOO" => "abc -100.4", "BAR" => "abc 0.5");
        $ro = $p->opStringStringD($di1, $di2, $_do);
        test(count($_do) == 2);
        test($_do["foo"] == $di1["foo"]);
        test($_do["bar"] == $di1["bar"]);
        test(count($ro) == 4);
        test($ro["foo"] == "abc -1.1");
        test($ro["FOO"] == "abc -100.4");
        test($ro["bar"] == "abc 123123.2");
        test($ro["BAR"] == "abc 0.5");
    }

    {
        $di1 = array("abc" => Test_MyEnum::enum1, "" => Test_MyEnum::enum2);
        $di2 = array("abc" => Test_MyEnum::enum1, "qwerty" => Test_MyEnum::enum3, "Hello!!" => Test_MyEnum::enum2);
        $ro = $p->opStringMyEnumD($di1, $di2, $_do);
        test(count($_do) == 2);
        test($_do["abc"] == $di1["abc"]);
        test($_do[""] == $di1[""]);
        test(count($ro) == 4);
        test($ro["abc"] == Test_MyEnum::enum1);
        test($ro["qwerty"] == Test_MyEnum::enum3);
        test($ro[""] == Test_MyEnum::enum2);
        test($ro["Hello!!"] == Test_MyEnum::enum2);
    }

    {
        $lengths = array(0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000);
        foreach($lengths as $l)
        {
            $s = array();
            for($i = 0; $i < $l; $i++)
            {
                $s[$i] = $i;
            }
            $r = $p->opIntS($s);
            test(count($r) == $l);
            for($j = 0; $j < count($r); $j++)
            {
                test($r[$j] == -$j);
            }
        }
    }
}

function allTests()
{
    global $ICE;

    $ref = "test:default -p 12010 -t 2000";
    $base = $ICE->stringToProxy($ref);
    $cl = $base->ice_checkedCast("::Test::MyClass");
    $derived = $cl->ice_checkedCast("::Test::MyDerivedClass");

    echo "testing twoway operations... ";
    flush();
    twoways($ICE, $cl);
    twoways($ICE, $derived);
    $derived->opDerived();
    echo "ok\n";

    return $cl;
}

$myClass = allTests();

echo "testing server shutdown... ";
flush();
$myClass->shutdown();
try
{
    $myClass->opVoid();
    test(false);
}
catch(Ice_LocalException $ex)
{
    echo "ok\n";
}

exit();
?>
