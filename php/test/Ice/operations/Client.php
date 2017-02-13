<?php
// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
require_once('Ice.php');
require_once('Test.php');

function test($b)
{
    if(!$b)
    {
        $bt = debug_backtrace();
        echo "\ntest failed in ".$bt[0]["file"]." line ".$bt[0]["line"]."\n";
        exit(1);
    }
}

function twoways($communicator, $p)
{
    global $NS;

    $enum1 = $NS ? constant("Test\\MyEnum::enum1") : constant("Test_MyEnum::enum1");
    $enum2 = $NS ? constant("Test\\MyEnum::enum2") : constant("Test_MyEnum::enum2");
    $enum3 = $NS ? constant("Test\\MyEnum::enum3") : constant("Test_MyEnum::enum3");

    $myClassPrxHelper = $NS ? "Test\\MyClassPrxHelper" : "Test_MyClassPrxHelper";
    $myDerivedClassPrxHelper = $NS ? "Test\\MyDerivedClassPrxHelper" : "Test_MyDerivedClassPrxHelper";
    $objectPrxHelper = $NS ? "Ice\\ObjectPrxHelper" : "Ice_ObjectPrxHelper";


    $s0 = $NS ? constant("Test\\s0") : constant("Test_s0");
    $s1 = $NS ? constant("Test\\s1") : constant("Test_s1");
    $s2 = $NS ? constant("Test\\s2") : constant("Test_s2");
    $s3 = $NS ? constant("Test\\s3") : constant("Test_s3");
    $s4 = $NS ? constant("Test\\s4") : constant("Test_s4");
    $s5 = $NS ? constant("Test\\s5") : constant("Test_s5");
    $s6 = $NS ? constant("Test\\s6") : constant("Test_s6");
    $s7 = $NS ? constant("Test\\s7") : constant("Test_s7");
    $s8 = $NS ? constant("Test\\s8") : constant("Test_s8");
    $s9 = $NS ? constant("Test\\s9") : constant("Test_s9");
    $s10 = $NS ? constant("Test\\s10") : constant("Test_s10");

    $sw0 = $NS ? constant("Test\\sw0") : constant("Test_sw0");
    $sw1 = $NS ? constant("Test\\sw1") : constant("Test_sw1");
    $sw2 = $NS ? constant("Test\\sw2") : constant("Test_sw2");
    $sw3 = $NS ? constant("Test\\sw3") : constant("Test_sw3");
    $sw4 = $NS ? constant("Test\\sw4") : constant("Test_sw4");
    $sw5 = $NS ? constant("Test\\sw5") : constant("Test_sw5");
    $sw6 = $NS ? constant("Test\\sw6") : constant("Test_sw6");
    $sw7 = $NS ? constant("Test\\sw7") : constant("Test_sw7");
    $sw8 = $NS ? constant("Test\\sw8") : constant("Test_sw8");
    $sw9 = $NS ? constant("Test\\sw9") : constant("Test_sw9");
    $sw10 = $NS ? constant("Test\\sw10") : constant("Test_sw10");

    $ss0 = $NS ? constant("Test\\ss0") : constant("Test_ss0");
    $ss1 = $NS ? constant("Test\\ss1") : constant("Test_ss1");
    $ss2 = $NS ? constant("Test\\ss2") : constant("Test_ss2");
    $ss3 = $NS ? constant("Test\\ss3") : constant("Test_ss3");
    $ss4 = $NS ? constant("Test\\ss4") : constant("Test_ss4");
    $ss5 = $NS ? constant("Test\\ss5") : constant("Test_ss5");

    $su0 = $NS ? constant("Test\\su0") : constant("Test_su0");
    $su1 = $NS ? constant("Test\\su1") : constant("Test_su1");
    $su2 = $NS ? constant("Test\\su2") : constant("Test_su2");

    {
        $literals = $p->opStringLiterals();

        test($s0 == "\\" &&
             $s0 == $sw0 &&
             $s0 == $literals[0] &&
             $s0 == $literals[11]);

        test($s1 == "A" &&
             $s1 == $sw1 &&
             $s1 == $literals[1] &&
             $s1 == $literals[12]);

        test($s2 == "Ice" &&
             $s2 == $sw2 &&
             $s2 == $literals[2] &&
             $s2 == $literals[13]);

        test($s3 == "A21" &&
             $s3 == $sw3 &&
             $s3 == $literals[3] &&
             $s3 == $literals[14]);

        test($s4 == "\\u0041 \\U00000041" &&
             $s4 == $sw4 &&
             $s4 == $literals[4] &&
             $s4 == $literals[15]);

        test($s5 == "\xc3\xbf" &&
             $s5 == $sw5 &&
             $s5 == $literals[5] &&
             $s5 == $literals[16]);

        test($s6 == "\xcf\xbf" &&
             $s6 == $sw6 &&
             $s6 == $literals[6] &&
             $s6 == $literals[17]);

        test($s7 == "\xd7\xb0" &&
             $s7 == $sw7 &&
             $s7 == $literals[7] &&
             $s7 == $literals[18]);

        test($s8 == "\xf0\x90\x80\x80" &&
             $s8 == $sw8 &&
             $s8 == $literals[8] &&
             $s8 == $literals[19]);

        test($s9 == "\xf0\x9f\x8d\x8c" &&
             $s9 == $sw9 &&
             $s9 == $literals[9] &&
             $s9 == $literals[20]);

        test($s10 == "\xe0\xb6\xa7" &&
             $s10 == $sw10 &&
             $s10 == $literals[10] &&
             $s10 == $literals[21]);

        test($ss0 == "'\"?\\\007\010\f\n\r\t\v\6" &&
             $ss0 == $ss1 &&
             $ss1 == $ss2 &&
             $ss0 == $literals[22] &&
             $ss0 == $literals[23] &&
             $ss0 == $literals[24]);

        test($ss3 == "\\\\U\\u\\" &&
             $ss3 == $literals[25]);

        test($ss4 == "\\A\\" &&
             $ss4 == $literals[26]);

        test($ss5 == "\\u0041\\" &&
             $ss5 == $literals[27]);

        test($su0 == $su1 &&
             $su0 == $su2 &&
             $su0 == $literals[28] &&
             $su0 == $literals[29] &&
             $su0 == $literals[30]);
    }

    {
        $p->ice_ping();
    }

    {
        test($objectPrxHelper::ice_staticId() == "::Ice::Object");
    }

    {
        test($p->ice_isA($myClassPrxHelper::ice_staticId()));
    }

    {
        test($p->ice_id() == $myDerivedClassPrxHelper::ice_staticId());
    }

    {
        test(count($p->ice_ids()) == 3);
    }

    {
        test($p->ice_id() == "::Test::MyDerivedClass");
    }

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
        //
        // Test invalid ranges for numbers.
        //
        try
        {
            $r = $p->opByte(0x01ff, 0x0f, $b);
            test(false);
        }
        catch(InvalidArgumentException $ex)
        {
        }

        try
        {
            $r = $p->opShortIntLong($SHORT_MAX + 1, 0, 0, $s, $i, $l);
            test(false);
        }
        catch(InvalidArgumentException $ex)
        {
        }

        try
        {
            $r = $p->opShortIntLong($SHORT_MIN - 1, 0, 0, $s, $i, $l);
            test(false);
        }
        catch(InvalidArgumentException $ex)
        {
        }

        try
        {
            $r = $p->opShortIntLong(0, $INT_MAX + 1, 0, $s, $i, $l);
            test(false);
        }
        catch(InvalidArgumentException $ex)
        {
        }

        try
        {
            $r = $p->opShortIntLong(0, $INT_MIN - 1, 0, $s, $i, $l);
            test(false);
        }
        catch(InvalidArgumentException $ex)
        {
        }

        try
        {
            $r = $p->opShortIntLong(0, 0, $LONG_MAX + 1, $s, $i, $l);
            test(false);
        }
        catch(InvalidArgumentException $ex)
        {
        }

        try
        {
            $r = $p->opShortIntLong(0, 0, $LONG_MIN - 1, $s, $i, $l);
            test(false);
        }
        catch(InvalidArgumentException $ex)
        {
        }

        $r = $p->opFloatDouble(3.402823466E38, 0.0, $f, $d);
        $r = $p->opFloatDouble(-3.402823466E38, 0.0, $f, $d);

        foreach(array(NAN, -NAN) as $val)
        {
            $r = $p->opFloatDouble($val, $val, $f, $d);
            test(is_nan($r) && is_nan($f) && is_nan($d));
        }
        foreach(array(INF, -INF) as $val)
        {
            $r = $p->opFloatDouble($val, $val, $f, $d);
            test(is_infinite($r) && is_infinite($f) && is_infinite($d));
        }

        try
        {
            $r = $p->opFloatDouble(3.402823466E38*2, 0.0, $f, $d);
            test(false);
        }
        catch(InvalidArgumentException $ex)
        {
        }

        try
        {
            $r = $p->opFloatDouble(-3.402823466E38*2, 0.0, $f, $d);
            test(false);
        }
        catch(InvalidArgumentException $ex)
        {
        }
    }

    {
        //
        // Verify that null is accepted for strings.
        //
        $r = $p->opString(null, null, $s);
        test($s == " ");
        test($r == " ");
    }

    {
        $r = $p->opString("hello", "world", $s);
        test($s == "world hello");
        test($r == "hello world");
    }

    {
        $r = $p->opMyEnum($enum2, $e);
        test($e == $enum2);
        test($r == $enum3);
    }

    {
        $stringToIdentity = $NS ? "Ice\\stringToIdentity" : "Ice_stringToIdentity";

        $r = $p->opMyClass($p, $c1, $c2);
        test(Ice_proxyIdentityAndFacetEqual($c1, $p));
        test(!Ice_proxyIdentityAndFacetEqual($c2, $p));
        test(Ice_proxyIdentityAndFacetEqual($r, $p));
        test($c1->ice_getIdentity() == $stringToIdentity("test"));
        test($c2->ice_getIdentity() == $stringToIdentity("noSuchIdentity"));
        test($r->ice_getIdentity() == $stringToIdentity("test"));
        $r->opVoid();
        $c1->opVoid();
        try
        {
            $c2->opVoid();
            test(false);
        }
        catch(Exception $ex)
        {
            $le = $NS ? "Ice\\LocalException" : "Ice_LocalException";
            if(!($ex instanceof $le))
            {
                throw $ex;
            }
        }

        $r = $p->opMyClass(null, $c1, $c2);
        test($c1 == null);
        test($c2 != null);
        $r->opVoid();
    }

    {
        $si1 = $NS ? eval("return new Test\\Structure;") : eval("return new Test_Structure;");
        $si1->p = $p;
        $si1->e = $enum3;
        $si1->s = $NS ? eval("return new Test\\AnotherStruct;") : eval("return new Test_AnotherStruct;");
        $si1->s->s = "abc";
        $si2 = $NS ? eval("return new Test\\Structure;") : eval("return new Test_Structure;");
        $si2->p = null;
        $si2->e = $enum2;
        $si2->s = $NS ? eval("return new Test\\AnotherStruct;") : eval("return new Test_AnotherStruct;");
        $si2->s->s = "def";

        $rso = $p->opStruct($si1, $si2, $so);
        test($rso->p == null);
        test($rso->e == $enum2);
        test($rso->s->s == "def");
        test($so->p == $p);
        test($so->e == $enum3);
        test($so->s->s == "a new string");
        $so->p->opVoid();

        // Test marshalling of null structs and structs with null members.
        $si1 = $NS ? eval("return new Test\\Structure;") : eval("return new Test_Structure;");
        $si2 = null;

        $rso = $p->opStruct($si1, $si2, $so);
        test($rso->p == null);
        test($rso->e == $enum1);
        test($rso->s->s == "");
        test($so->p == null);
        test($so->e == $enum1);
        test($so->s->s == "a new string");
    }

    {
        //
        // Verify that null is accepted for sequences.
        //
        $rso = $p->opByteS(null, null, $bso);
        test(count($bso) == 0);
        test(count($rso) == 0);
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
        //
        // Verify that null is accepted for dictionaries.
        //
        $ro = $p->opByteBoolD(null, null, $_do);
        test(count($_do) == 0);
        test(count($ro) == 0);
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
        $di1 = array("abc" => $enum1, "" => $enum2);
        $di2 = array("abc" => $enum1, "qwerty" => $enum3, "Hello!!" => $enum2);
        $ro = $p->opStringMyEnumD($di1, $di2, $_do);
        test(count($_do) == 2);
        test($_do["abc"] == $di1["abc"]);
        test($_do[""] == $di1[""]);
        test(count($ro) == 4);
        test($ro["abc"] == $enum1);
        test($ro["qwerty"] == $enum3);
        test($ro[""] == $enum2);
        test($ro["Hello!!"] == $enum2);
    }

    {
        $di1 = array($enum1 => "abc");
        $di2 = array($enum2 => "Hello!!", $enum3 => "qwerty");
        $ro = $p->opMyEnumStringD($di1, $di2, $_do);
        test(count($_do) == 1);
        test($_do[$enum1] == $di1[$enum1]);
        test(count($ro) == 3);
        test($ro[$enum1] == "abc");
        test($ro[$enum2] == "Hello!!");
        test($ro[$enum3] == "qwerty");
    }

    {
        $dsi1 = array(array(10 => true, 100 => false ), array(10 => true, 11 => false, 101 => true));
        $dsi2 = array(array(100 => false, 101 => false));

        $ro = $p->opByteBoolDS($dsi1, $dsi2, $_do);

        test(count($ro) == 2);
        test(count($ro[0]) == 3);
        test($ro[0][10]);
        test(!$ro[0][11]);
        test($ro[0][101]);
        test(count($ro[1]) == 2);
        test($ro[1][10]);
        test(!$ro[1][100]);
        test(count($_do) == 3);
        test(count($_do[0]) == 2);
        test(!$_do[0][100]);
        test(!$_do[0][101]);
        test(count($_do[1]) == 2);
        test($_do[1][10]);
        test(!$_do[1][100]);
        test(count($_do[2]) == 3);
        test($_do[2][10]);
        test(!$_do[2][11]);
        test($_do[2][101]);
    }

    {
        $dsi1 = array(array(110 => -1, 1100 => 123123), array(110 => -1, 111 => -100, 1101 => 0));
        $dsi2 = array(array(100 => -1001));

        $ro= $p->opShortIntDS($dsi1, $dsi2, $_do);

        test(count($ro) == 2);
        test(count($ro[0]) == 3);
        test($ro[0][110] == -1);
        test($ro[0][111] == -100);
        test($ro[0][1101] == 0);
        test(count($ro[1]) == 2);
        test($ro[1][110] == -1);
        test($ro[1][1100] == 123123);

        test(count($_do) == 3);
        test(count($_do[0]) == 1);
        test($_do[0][100] == -1001);
        test(count($_do[1]) == 2);
        test($_do[1][110] == -1);
        test($_do[1][1100] == 123123);
        test(count($_do[2]) == 3);
        test($_do[2][110] == -1);
        test($_do[2][111] == -100);
        test($_do[2][1101] == 0);
    }

    {
        $dsi1 = array(array(999999110 => -1.1, 999999111 => 123123.2 ),
                      array(999999110 => -1.1, 999999120 => -100.4, 999999130 => 0.5 ));
        $dsi2 = array(array(999999140 => 3.14 ));

        $ro = $p->opLongFloatDS($dsi1, $dsi2, $_do);

        test(count($ro) == 2);
        test(count($ro[0]) == 3);
        test($ro[0][999999110] - -1.1 < 0.01);
        test($ro[0][999999120] - -100.4 < 0.01);
        test($ro[0][999999130] - 0.5 < 0.01);
        test(count($ro[1]) == 2);
        test($ro[1][999999110] - -1.1 < 0.01);
        test($ro[1][999999111] - 123123.2 < 0.01);

        test(count($_do) == 3);
        test(count($_do[0]) == 1);
        test($_do[0][999999140] - 3.14 < 0.01);
        test(count($_do[1]) == 2);
        test($_do[1][999999110] - -1.1 < 0.01);
        test($_do[1][999999111] - 123123.2 < 0.01);
        test(count($_do[2]) == 3);
        test($_do[2][999999110] - -1.1 < 0.01);
        test($_do[2][999999120] - -100.4 < 0.01);
        test($_do[2][999999130] - 0.5 < 0.01);
    }

    {
        $dsi1 = array(array("foo" => "abc -1.1", "bar" => "abc 123123.2" ),
                      array("foo" => "abc -1.1", "FOO" => "abc -100.4", "BAR" => "abc 0.5" ));
        $dsi2 = array(array("f00" => "ABC -3.14" ));

        $ro = $p->opStringStringDS($dsi1, $dsi2, $_do);

        test(count($ro) == 2);
        test(count($ro[0]) == 3);
        test($ro[0]["foo"] == "abc -1.1");
        test($ro[0]["FOO"] == "abc -100.4");
        test($ro[0]["BAR"] == "abc 0.5");
        test(count($ro[1]) == 2);
        test($ro[1]["foo"] == "abc -1.1");
        test($ro[1]["bar"] == "abc 123123.2");

        test(count($_do) == 3);
        test(count($_do[0]) == 1);
        test($_do[0]["f00"] == "ABC -3.14");
        test(count($_do[1]) == 2);
        test($_do[1]["foo"] == "abc -1.1");
        test($_do[1]["bar"] == "abc 123123.2");
        test(count($_do[2]) == 3);
        test($_do[2]["foo"] == "abc -1.1");
        test($_do[2]["FOO"] == "abc -100.4");
        test($_do[2]["BAR"] == "abc 0.5");
    }

    {
        $dsi1 = array(array("abc" => $enum1, "" => $enum2),
                     array("abc" => $enum1, "qwerty" => $enum3, "Hello!!" => $enum2));
        $dsi2 = array(array("Goodbye" => $enum1));

        $ro = $p->opStringMyEnumDS($dsi1, $dsi2, $_do);

        test(count($ro) == 2);
        test(count($ro[0]) == 3);
        test($ro[0]["abc"] == $enum1);
        test($ro[0]["qwerty"] == $enum3);
        test($ro[0]["Hello!!"] == $enum2);
        test(count($ro[1]) == 2);
        test($ro[1]["abc"] == $enum1);
        test($ro[1][""] == $enum2);

        test(count($_do) == 3);
        test(count($_do[0]) == 1);
        test($_do[0]["Goodbye"] == $enum1);
        test(count($_do[1]) == 2);
        test($_do[1]["abc"] == $enum1);
        test($_do[1][""] == $enum2);
        test(count($_do[2]) == 3);
        test($_do[2]["abc"] == $enum1);
        test($_do[2]["qwerty"] == $enum3);
        test($_do[2]["Hello!!"] == $enum2);
    }

    {
        $dsi1 = array(array($enum1 => 'abc'), array($enum2 => 'Hello!!', $enum3 => 'qwerty'));
        $dsi2 = array(array($enum1 => 'Goodbye'));

        $ro = $p->opMyEnumStringDS($dsi1, $dsi2, $_do);

        test(count($ro) == 2);
        test(count($ro[0]) == 2);
        test($ro[0][$enum2] == "Hello!!");
        test($ro[0][$enum3] == "qwerty");
        test(count($ro[1]) == 1);
        test($ro[1][$enum1] == "abc");

        test(count($_do) == 3);
        test(count($_do[0]) == 1);
        test($_do[0][$enum1] == "Goodbye");
        test(count($_do[1]) == 1);
        test($_do[1][$enum1] == "abc");
        test(count($_do[2]) == 2);
        test($_do[2][$enum2] == "Hello!!");
        test($_do[2][$enum3] == "qwerty");
    }

    {
        $sdi1 = array(0x01 => array(0x01, 0x11), 0x22 => array(0x12));
        $sdi2 = array(0xf1 => array(0xf2, 0xf3));

        $ro = $p->opByteByteSD($sdi1, $sdi2, $_do);

        test($_do == $sdi2);
        test(count($ro) == 3);
        test(count($ro[0x01]) == 2);
        test($ro[0x01][0] == 0x01);
        test($ro[0x01][1] == 0x11);
        test(count($ro[0x22]) == 1);
        test($ro[0x22][0] == 0x12);
        test(count($ro[0xf1]) == 2);
        test($ro[0xf1][0] == 0xf2);
        test($ro[0xf1][1] == 0xf3);
    }

    {
        $sdi1 = array(false => array(true, false), true => array(false, true, true));
        $sdi2 = array(false => array(true, false));

        $ro = $p->opBoolBoolSD($sdi1, $sdi2, $_do);

        test($_do == $sdi2);
        test(count($ro) == 2);
        test(count($ro[false]) == 2);
        test($ro[false][0]);
        test(!$ro[false][1]);
        test(count($ro[true]) == 3);
        test(!$ro[true][0]);
        test($ro[true][1]);
        test($ro[true][2]);
    }

    {
        $sdi1 = array(1 => array(1, 2, 3), 2 => array(4, 5));
        $sdi2 = array(4 => array(6, 7));

        $ro = $p->opShortShortSD($sdi1, $sdi2, $_do);

        test($_do == $sdi2);
        test(count($ro) == 3);
        test(count($ro[1]) == 3);
        test($ro[1][0] == 1);
        test($ro[1][1] == 2);
        test($ro[1][2] == 3);
        test(count($ro[2]) == 2);
        test($ro[2][0] == 4);
        test($ro[2][1] == 5);
        test(count($ro[4]) == 2);
        test($ro[4][0] == 6);
        test($ro[4][1] == 7);
    }

    {
        $sdi1 = array(100 => array(100, 200, 300), 200 => array(400, 500));
        $sdi2 = array(400 => array(600, 700));

        $ro = $p->opIntIntSD($sdi1, $sdi2, $_do);

        test($_do == $sdi2);
        test(count($ro) == 3);
        test(count($ro[100]) == 3);
        test($ro[100][0] == 100);
        test($ro[100][1] == 200);
        test($ro[100][2] == 300);
        test(count($ro[200]) == 2);
        test($ro[200][0] == 400);
        test($ro[200][1] == 500);
        test(count($ro[400]) == 2);
        test($ro[400][0] == 600);
        test($ro[400][1] == 700);
    }

    {
        $sdi1 = array(999999990 => array(999999110, 999999111, 999999110), 999999991 => array(999999120, 999999130));
        $sdi2 = array(999999992 => array(999999110, 999999120));

        $ro = $p->opLongLongSD($sdi1, $sdi2, $_do);

        test($_do == $sdi2);
        test(count($ro) == 3);
        test(count($ro[999999990]) == 3);
        test($ro[999999990][0] == 999999110);
        test($ro[999999990][1] == 999999111);
        test($ro[999999990][2] == 999999110);
        test(count($ro[999999991]) == 2);
        test($ro[999999991][0] == 999999120);
        test($ro[999999991][1] == 999999130);
        test(count($ro[999999992]) == 2);
        test($ro[999999992][0] == 999999110);
        test($ro[999999992][1] == 999999120);
    }

    {
        $sdi1 = array("abc" => array(-1.1, 123123.2, 100.0), "ABC" => array(42.24, -1.61));
        $sdi2 = array("aBc" => array(-3.14, 3.14));

        $ro = $p->opStringFloatSD($sdi1, $sdi2, $_do);

        test(count($_do) == 1);
        test(count($_do["aBc"]) == 2);
        test($_do["aBc"][0] - -3.14 < 0.01);
        test($_do["aBc"][1] - 3.14 < 0.01);
        test(count($ro) == 3);
        test(count($ro["abc"]) == 3);
        test($ro["abc"][0] - -1.1 < 0.01);
        test($ro["abc"][1] - 123123.2 < 0.01);
        test($ro["abc"][2] - 100.0 < 0.01);
        test(count($ro["ABC"]) == 2);
        test($ro["ABC"][0] - 42.24 < 0.01);
        test($ro["ABC"][1] - -1.61 < 0.01);
        test(count($ro["aBc"]) == 2);
        test($ro["aBc"][0] - -3.14 < 0.01);
        test($ro["aBc"][1] - 3.14 < 0.01);
    }

    {
        $sdi1 = array("Hello!!" => array(1.1E10, 1.2E10, 1.3E10), "Goodbye" => array(1.4E10, 1.5E10));
        $sdi2 = array("" => array(1.6E10, 1.7E10));

        $ro = $p->opStringDoubleSD($sdi1, $sdi2, $_do);

        test($_do == $sdi2);
        test(count($ro) == 3);
        test(count($ro["Hello!!"]) == 3);
        test($ro["Hello!!"][0] == 1.1E10);
        test($ro["Hello!!"][1] == 1.2E10);
        test($ro["Hello!!"][2] == 1.3E10);
        test(count($ro["Goodbye"]) == 2);
        test($ro["Goodbye"][0] == 1.4E10);
        test($ro["Goodbye"][1] == 1.5E10);
        test(count($ro[""]) == 2);
        test($ro[""][0] == 1.6E10);
        test($ro[""][1] == 1.7E10);
    }

    {
        $sdi1 = array("abc" => array("abc", "de", "fghi") , "def" => array("xyz", "or"));
        $sdi2 = array("ghi" => array("and", "xor"));

        $ro = $p->opStringStringSD($sdi1, $sdi2, $_do);

        test($_do == $sdi2);
        test(count($ro) == 3);
        test(count($ro["abc"]) == 3);
        test($ro["abc"][0] == "abc");
        test($ro["abc"][1] == "de");
        test($ro["abc"][2] == "fghi");
        test(count($ro["def"]) == 2);
        test($ro["def"][0] == "xyz");
        test($ro["def"][1] == "or");
        test(count($ro["ghi"]) == 2);
        test($ro["ghi"][0] == "and");
        test($ro["ghi"][1] == "xor");
    }

    {
        $sdi1 = array($enum3 => array($enum1, $enum1, $enum2), $enum2 => array($enum1, $enum2));
        $sdi2 = array($enum1 => array($enum3, $enum3));

        $ro = $p->opMyEnumMyEnumSD($sdi1, $sdi2, $_do);

        test($_do == $sdi2);
        test(count($ro) == 3);
        test(count($ro[$enum3]) == 3);
        test($ro[$enum3][0] == $enum1);
        test($ro[$enum3][1] == $enum1);
        test($ro[$enum3][2] == $enum2);
        test(count($ro[$enum2]) == 2);
        test($ro[$enum2][0] == $enum1);
        test($ro[$enum2][1] == $enum2);
        test(count($ro[$enum1]) == 2);
        test($ro[$enum1][0] == $enum3);
        test($ro[$enum1][1] == $enum3);
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

    {
        $p->opIdempotent();
    }

    {
        $p->opNonmutating();
    }

    test($p->opByte1(0xFF) == 0xFF);
    test($p->opShort1(0x7FFF) == 0x7FFF);
    test($p->opInt1(0x7FFFFFFF) == 0x7FFFFFFF);
    test($p->opLong1($LONG_MAX) == $LONG_MAX);
    test($p->opFloat1(1.0) == 1.0);
    test($p->opDouble1(1.0) == 1.0);
    test($p->opString1("opString1") == "opString1");
    test(count($p->opStringS1(null)) == 0);
    test(count($p->opByteBoolD1(null)) == 0);
    test(count($p->opStringS2(null)) == 0);
    test(count($p->opByteBoolD2(null)) == 0);

    {
        $p1 = $p->opMStruct1();
        $p1->e = $enum3;
        $p2 = null;
        $p3 = $p->opMStruct2($p1, $p2);
        test($p2 == $p1 && $p3 == $p1);
    }

    {
        $p->opMSeq1();
        $p1 = array("test");
        $p2 = null;
        $p3 = $p->opMSeq2($p1, $p2);
        test($p2[0] == "test" && $p3[0] == "test");
    }

    {
        $p->opMDict1();
        $p1 = array("test" => "test");
        $p2 = null;
        $p3 = $p->opMDict2($p1, $p2);
        test($p3["test"] == "test" && $p2["test"] == "test");
    }
}

function allTests($communicator)
{
    global $NS;

    $ref = "test:default -p 12010";
    $base = $communicator->stringToProxy($ref);
    $cl = $base->ice_checkedCast("::Test::MyClass");
    $derived = $cl->ice_checkedCast("::Test::MyDerivedClass");

    echo "testing twoway operations... ";
    flush();
    twoways($communicator, $cl);
    twoways($communicator, $derived);
    $derived->opDerived();
    echo "ok\n";

    # Test flush batch requests methods
    $BasedOnProxy = $NS ? constant("Ice\\CompressBatch::BasedOnProxy") : constant("Ice_CompressBatch::BasedOnProxy");

    $derived->ice_flushBatchRequests();
    $derived->ice_getConnection()->flushBatchRequests($BasedOnProxy);
    $derived->ice_getCommunicator()->flushBatchRequests($BasedOnProxy);

    return $cl;
}

$communicator = $NS ? eval("return Ice\\initialize(\$argv);") :
                      eval("return Ice_initialize(\$argv);");

$myClass = allTests($communicator);

echo "testing server shutdown... ";
flush();
$myClass->shutdown();
try
{
    $myClass->opVoid();
    test(false);
}
catch(Exception $ex)
{
    $le = $NS ? "Ice\\LocalException" : "Ice_LocalException";
    if(!($ex instanceof $le))
    {
        throw $ex;
    }
    echo "ok\n";
}

# Test multiple destroy calls
$communicator->destroy();
$communicator->destroy();

$communicator->destroy();

exit();
?>
