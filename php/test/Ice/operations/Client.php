<?php
// Copyright (c) ZeroC, Inc.

require_once('Test.php');

function twoways($communicator, $p)
{
    {
        $literals = $p->opStringLiterals();

        test(Test\s0 == "\\" &&
             Test\s0 == Test\sw0 &&
             Test\s0 == $literals[0] &&
             Test\s0 == $literals[11]);

        test(Test\s1 == "A" &&
             Test\s1 == Test\sw1 &&
             Test\s1 == $literals[1] &&
             Test\s1 == $literals[12]);

        test(Test\s2 == "Ice" &&
             Test\s2 == Test\sw2 &&
             Test\s2 == $literals[2] &&
             Test\s2 == $literals[13]);

        test(Test\s3 == "A21" &&
             Test\s3 == Test\sw3 &&
             Test\s3 == $literals[3] &&
             Test\s3 == $literals[14]);

        test(Test\s4 == "\\u0041 \\U00000041" &&
             Test\s4 == Test\sw4 &&
             Test\s4 == $literals[4] &&
             Test\s4 == $literals[15]);

        test(Test\s5 == "\xc3\xbf" &&
             Test\s5 == Test\sw5 &&
             Test\s5 == $literals[5] &&
             Test\s5 == $literals[16]);

        test(Test\s6 == "\xcf\xbf" &&
             Test\s6 == Test\sw6 &&
             Test\s6 == $literals[6] &&
             Test\s6 == $literals[17]);

        test(Test\s7 == "\xd7\xb0" &&
             Test\s7 == Test\sw7 &&
             Test\s7 == $literals[7] &&
             Test\s7 == $literals[18]);

        test(Test\s8 == "\xf0\x90\x80\x80" &&
             Test\s8 == Test\sw8 &&
             Test\s8 == $literals[8] &&
             Test\s8 == $literals[19]);

        test(Test\s9 == "\xf0\x9f\x8d\x8c" &&
             Test\s9 == Test\sw9 &&
             Test\s9 == $literals[9] &&
             Test\s9 == $literals[20]);

        test(Test\s10 == "\xe0\xb6\xa7" &&
             Test\s10 == Test\sw10 &&
             Test\s10 == $literals[10] &&
             Test\s10 == $literals[21]);

        test(Test\ss0 == "'\"?\\\007\010\f\n\r\t\v\6" &&
             Test\ss0 == Test\ss1 &&
             Test\ss1 == Test\ss2 &&
             Test\ss0 == $literals[22] &&
             Test\ss0 == $literals[23] &&
             Test\ss0 == $literals[24]);

        test(Test\ss3 == "\\\\U\\u\\" &&
             Test\ss3 == $literals[25]);

        test(Test\ss4 == "\\A\\" &&
             Test\ss4 == $literals[26]);

        test(Test\ss5 == "\\u0041\\" &&
             Test\ss5 == $literals[27]);

        test(Test\su0 == Test\su1 &&
             Test\su0 == Test\su2 &&
             Test\su0 == $literals[28] &&
             Test\su0 == $literals[29] &&
             Test\su0 == $literals[30]);
    }

    {
        $p->ice_ping();
    }

    {
        test(Ice\ObjectPrxHelper::ice_staticId() == "::Ice::Object");
    }

    {
        test($p->ice_isA(Test\MyClassPrxHelper::ice_staticId()));
    }

    {
        test($p->ice_id() == Test\MyDerivedClassPrxHelper::ice_staticId());
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
        $r = $p->opMyEnum(Test\MyEnum::enum2, $e);
        test($e == Test\MyEnum::enum2);
        test($r == Test\MyEnum::enum3);
    }

    {
        $r = $p->opMyClass($p, $c1, $c2);
        test(Ice\proxyIdentityAndFacetEqual($c1, $p));
        test(!Ice\proxyIdentityAndFacetEqual($c2, $p));
        test(Ice\proxyIdentityAndFacetEqual($r, $p));
        test($c1->ice_getIdentity() == Ice\stringToIdentity("test"));
        test($c2->ice_getIdentity() == Ice\stringToIdentity("noSuchIdentity"));
        test($r->ice_getIdentity() == Ice\stringToIdentity("test"));
        $r->opVoid();
        $c1->opVoid();
        try
        {
            $c2->opVoid();
            test(false);
        }
        catch(Ice\LocalException $ex)
        {
        }

        $r = $p->opMyClass(null, $c1, $c2);
        test($c1 == null);
        test($c2 != null);
        $r->opVoid();
    }

    {
        $si1 = new Test\Structure();
        $si1->p = $p;
        $si1->e = Test\MyEnum::enum3;
        $si1->s = new Test\AnotherStruct();
        $si1->s->s = "abc";
        $si2 = new Test\Structure();
        $si2->p = null;
        $si2->e = Test\MyEnum::enum2;
        $si2->s = new Test\AnotherStruct();
        $si2->s->s = "def";

        $rso = $p->opStruct($si1, $si2, $so);
        test($rso->p == null);
        test($rso->e == Test\MyEnum::enum2);
        test($rso->s->s == "def");
        test($so->p == $p);
        test($so->e == Test\MyEnum::enum3);
        test($so->s->s == "a new string");
        $so->p->opVoid();

        // Test marshaling of null structs and structs with null members.
        $si1 = new Test\Structure();
        $si2 = null;

        $rso = $p->opStruct($si1, $si2, $so);
        test($rso->p == null);
        test($rso->e == Test\MyEnum::enum1);
        test($rso->s->s == "");
        test($so->p == null);
        test($so->e == Test\MyEnum::enum1);
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
        $di1 = array("abc" => Test\MyEnum::enum1, "" => Test\MyEnum::enum2);
        $di2 = array("abc" => Test\MyEnum::enum1, "qwerty" => Test\MyEnum::enum3, "Hello!!" => Test\MyEnum::enum2);
        $ro = $p->opStringMyEnumD($di1, $di2, $_do);
        test(count($_do) == 2);
        test($_do["abc"] == $di1["abc"]);
        test($_do[""] == $di1[""]);
        test(count($ro) == 4);
        test($ro["abc"] == Test\MyEnum::enum1);
        test($ro["qwerty"] == Test\MyEnum::enum3);
        test($ro[""] == Test\MyEnum::enum2);
        test($ro["Hello!!"] == Test\MyEnum::enum2);
    }

    {
        $di1 = array(Test\MyEnum::enum1 => "abc");
        $di2 = array(Test\MyEnum::enum2 => "Hello!!", Test\MyEnum::enum3 => "qwerty");
        $ro = $p->opMyEnumStringD($di1, $di2, $_do);
        test(count($_do) == 1);
        test($_do[Test\MyEnum::enum1] == $di1[Test\MyEnum::enum1]);
        test(count($ro) == 3);
        test($ro[Test\MyEnum::enum1] == "abc");
        test($ro[Test\MyEnum::enum2] == "Hello!!");
        test($ro[Test\MyEnum::enum3] == "qwerty");
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
        $dsi1 = array(array("abc" => Test\MyEnum::enum1, "" => Test\MyEnum::enum2),
                     array("abc" => Test\MyEnum::enum1, "qwerty" => Test\MyEnum::enum3, "Hello!!" => Test\MyEnum::enum2));
        $dsi2 = array(array("Goodbye" => Test\MyEnum::enum1));

        $ro = $p->opStringMyEnumDS($dsi1, $dsi2, $_do);

        test(count($ro) == 2);
        test(count($ro[0]) == 3);
        test($ro[0]["abc"] == Test\MyEnum::enum1);
        test($ro[0]["qwerty"] == Test\MyEnum::enum3);
        test($ro[0]["Hello!!"] == Test\MyEnum::enum2);
        test(count($ro[1]) == 2);
        test($ro[1]["abc"] == Test\MyEnum::enum1);
        test($ro[1][""] == Test\MyEnum::enum2);

        test(count($_do) == 3);
        test(count($_do[0]) == 1);
        test($_do[0]["Goodbye"] == Test\MyEnum::enum1);
        test(count($_do[1]) == 2);
        test($_do[1]["abc"] == Test\MyEnum::enum1);
        test($_do[1][""] == Test\MyEnum::enum2);
        test(count($_do[2]) == 3);
        test($_do[2]["abc"] == Test\MyEnum::enum1);
        test($_do[2]["qwerty"] == Test\MyEnum::enum3);
        test($_do[2]["Hello!!"] == Test\MyEnum::enum2);
    }

    {
        $dsi1 = array(array(Test\MyEnum::enum1 => 'abc'), array(Test\MyEnum::enum2 => 'Hello!!', Test\MyEnum::enum3 => 'qwerty'));
        $dsi2 = array(array(Test\MyEnum::enum1 => 'Goodbye'));

        $ro = $p->opMyEnumStringDS($dsi1, $dsi2, $_do);

        test(count($ro) == 2);
        test(count($ro[0]) == 2);
        test($ro[0][Test\MyEnum::enum2] == "Hello!!");
        test($ro[0][Test\MyEnum::enum3] == "qwerty");
        test(count($ro[1]) == 1);
        test($ro[1][Test\MyEnum::enum1] == "abc");

        test(count($_do) == 3);
        test(count($_do[0]) == 1);
        test($_do[0][Test\MyEnum::enum1] == "Goodbye");
        test(count($_do[1]) == 1);
        test($_do[1][Test\MyEnum::enum1] == "abc");
        test(count($_do[2]) == 2);
        test($_do[2][Test\MyEnum::enum2] == "Hello!!");
        test($_do[2][Test\MyEnum::enum3] == "qwerty");
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
        $sdi1 = array(Test\MyEnum::enum3 => array(Test\MyEnum::enum1, Test\MyEnum::enum1, Test\MyEnum::enum2), Test\MyEnum::enum2 => array(Test\MyEnum::enum1, Test\MyEnum::enum2));
        $sdi2 = array(Test\MyEnum::enum1 => array(Test\MyEnum::enum3, Test\MyEnum::enum3));

        $ro = $p->opMyEnumMyEnumSD($sdi1, $sdi2, $_do);

        test($_do == $sdi2);
        test(count($ro) == 3);
        test(count($ro[Test\MyEnum::enum3]) == 3);
        test($ro[Test\MyEnum::enum3][0] == Test\MyEnum::enum1);
        test($ro[Test\MyEnum::enum3][1] == Test\MyEnum::enum1);
        test($ro[Test\MyEnum::enum3][2] == Test\MyEnum::enum2);
        test(count($ro[Test\MyEnum::enum2]) == 2);
        test($ro[Test\MyEnum::enum2][0] == Test\MyEnum::enum1);
        test($ro[Test\MyEnum::enum2][1] == Test\MyEnum::enum2);
        test(count($ro[Test\MyEnum::enum1]) == 2);
        test($ro[Test\MyEnum::enum1][0] == Test\MyEnum::enum3);
        test($ro[Test\MyEnum::enum1][1] == Test\MyEnum::enum3);
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
        $p1->e = Test\MyEnum::enum3;
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

function allTests($helper)
{
    $ref = sprintf("test:%s", $helper->getTestEndpoint());
    $communicator = $helper->communicator();
    $cl = Test\MyClassPrxHelper::createProxy($communicator, $ref);
    $derived = Test\MyDerivedClassPrxHelper::checkedCast($cl);

    echo "testing twoway operations... ";
    flush();
    twoways($communicator, $cl);
    twoways($communicator, $derived);
    $derived->opDerived();
    echo "ok\n";

    # Test flush batch requests methods
    $derived->ice_flushBatchRequests();
    $derived->ice_getConnection()->flushBatchRequests(Ice\CompressBatch::BasedOnProxy);
    $derived->ice_getCommunicator()->flushBatchRequests(Ice\CompressBatch::BasedOnProxy);

    return $cl;
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
            try
            {
                $proxy->opVoid();
                test(false);
            }
            catch(Ice\LocalException $ex)
            {
            }
            # Test multiple destroy calls
            $communicator->destroy();
            $communicator->destroy();
        }
        catch(Exception $ex)
        {
            $communicator->destroy();
            throw $ex;
        }
    }
}
