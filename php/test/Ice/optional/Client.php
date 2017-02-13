<?php
// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
require_once('ClientPrivate.php');

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
    global $Ice_Encoding_1_0;

    $enum = $NS ? constant("Test\\MyEnum::MyEnumMember") : constant("Test_MyEnum::MyEnumMember");

    echo "testing stringToProxy... ";
    flush();
    $ref = "initial:default -p 12010";
    $base = $communicator->stringToProxy($ref);
    echo "ok\n";

    echo "testing checked cast... ";
    flush();
    $initial = $base->ice_checkedCast("::Test::Initial");
    echo "ok\n";

    echo "testing optional data members... ";
    flush();

    $oocls = $NS ? "Test\\OneOptional" : "Test_OneOptional";
    $oo1 = new $oocls;
    test($oo1->a == Ice_Unset);
    $oo1->a = 15;

    $oo2 = new $oocls(16);
    test($oo2->a == 16);

    $mocls = $NS ? "Test\\MultiOptional" : "Test_MultiOptional";
    $mo1 = new $mocls;
    test($mo1->a == Ice_Unset);
    test($mo1->b == Ice_Unset);
    test($mo1->c == Ice_Unset);
    test($mo1->d == Ice_Unset);
    test($mo1->e == Ice_Unset);
    test($mo1->f == Ice_Unset);
    test($mo1->g == Ice_Unset);
    test($mo1->h == Ice_Unset);
    test($mo1->i == Ice_Unset);
    test($mo1->j == Ice_Unset);
    test($mo1->k == Ice_Unset);
    test($mo1->bs == Ice_Unset);
    test($mo1->ss == Ice_Unset);
    test($mo1->iid == Ice_Unset);
    test($mo1->sid == Ice_Unset);
    test($mo1->fs == Ice_Unset);
    test($mo1->vs == Ice_Unset);

    test($mo1->shs == Ice_Unset);
    test($mo1->es == Ice_Unset);
    test($mo1->fss == Ice_Unset);
    test($mo1->vss == Ice_Unset);
    test($mo1->oos == Ice_Unset);
    test($mo1->oops == Ice_Unset);

    test($mo1->ied == Ice_Unset);
    test($mo1->ifsd == Ice_Unset);
    test($mo1->ivsd == Ice_Unset);
    test($mo1->iood == Ice_Unset);
    test($mo1->ioopd == Ice_Unset);

    test($mo1->bos == Ice_Unset);

    $sscls = $NS ? "Test\\SmallStruct" : "Test_SmallStruct";
    $ss = new $sscls();
    $fscls = $NS ? "Test\\FixedStruct" : "Test_FixedStruct";
    $fs = new $fscls(78);
    $vscls = $NS ? "Test\\VarStruct" : "Test_VarStruct";
    $vs = new $vscls("hello");
    $prx = $communicator->stringToProxy("test");
    $oo15 = new $oocls(15);
    $mocls = $NS ? "Test\\MultiOptional" : "Test_MultiOptional";
    $mo1 = new $mocls(15, true, 19, 78, 99, 5.5, 1.0, 'test', $enum,
                      $prx, null, array(5), array('test', 'test2'), array(4=>3), array('test'=>10),
                      $fs, $vs, array(1), array($enum, $enum), array($fs), array($vs), array($oo1),
                      array($prx), array(4=>$enum), array(4=>$fs), array(5=>$vs),
                      array(5=>$oo15), array(5=>$prx), array(false, true, false));

    test($mo1->a == 15);
    test($mo1->b == true);
    test($mo1->c == 19);
    test($mo1->d == 78);
    test($mo1->e == 99);
    test($mo1->f == 5.5);
    test($mo1->g == 1.0);
    test($mo1->h == "test");
    test($mo1->i == $enum);
    test($mo1->j == $prx);
    test($mo1->k == null);
    test($mo1->bs == array(5));
    test($mo1->ss == array("test", "test2"));
    test($mo1->iid[4] == 3);
    test($mo1->sid["test"] == 10);
    test($mo1->fs == $fs);
    test($mo1->vs == $vs);

    test($mo1->shs[0] == 1);
    test($mo1->es[0] == $enum && $mo1->es[1] == $enum);
    test($mo1->fss[0] == $fs);
    test($mo1->vss[0] == $vs);
    test($mo1->oos[0] == $oo1);
    test($mo1->oops[0] == $prx);

    test($mo1->ied[4] == $enum);
    test($mo1->ifsd[4] == $fs);
    test($mo1->ivsd[5] == $vs);
    test($mo1->iood[5]->a == 15);
    test($mo1->ioopd[5] == $prx);

    test($mo1->bos == array(false, true, false));
    
    
    //
    // Test generated struct and classes compare with Ice_Unset
    //
    test($ss != Ice_Unset);
    test($fs != Ice_Unset);
    test($vs != Ice_Unset);
    test($mo1 != Ice_Unset);

    echo "ok\n";

    echo "testing marshaling... ";
    flush();

    $oo4 = $initial->pingPong(new $oocls);
    test($oo4->a == Ice_Unset);

    $oo5 = $initial->pingPong($oo1);
    test($oo1->a == $oo5->a);

    $mo4 = $initial->pingPong(new $mocls);
    test($mo4->a == Ice_Unset);
    test($mo4->b == Ice_Unset);
    test($mo4->c == Ice_Unset);
    test($mo4->d == Ice_Unset);
    test($mo4->e == Ice_Unset);
    test($mo4->f == Ice_Unset);
    test($mo4->g == Ice_Unset);
    test($mo4->h == Ice_Unset);
    test($mo4->i == Ice_Unset);
    test($mo4->j == Ice_Unset);
    test($mo4->k == Ice_Unset);
    test($mo4->bs == Ice_Unset);
    test($mo4->ss == Ice_Unset);
    test($mo4->iid == Ice_Unset);
    test($mo4->sid == Ice_Unset);
    test($mo4->fs == Ice_Unset);
    test($mo4->vs == Ice_Unset);

    test($mo4->shs == Ice_Unset);
    test($mo4->es == Ice_Unset);
    test($mo4->fss == Ice_Unset);
    test($mo4->vss == Ice_Unset);
    test($mo4->oos == Ice_Unset);
    test($mo4->oops == Ice_Unset);

    test($mo4->ied == Ice_Unset);
    test($mo4->ifsd == Ice_Unset);
    test($mo4->ivsd == Ice_Unset);
    test($mo4->iood == Ice_Unset);
    test($mo4->ioopd == Ice_Unset);

    test($mo4->bos == Ice_Unset);

    $mo5 = $initial->pingPong($mo1);
    test($mo5->a == $mo1->a);
    test($mo5->b == $mo1->b);
    test($mo5->c == $mo1->c);
    test($mo5->d == $mo1->d);
    test($mo5->e == $mo1->e);
    test($mo5->f == $mo1->f);
    test($mo5->g == $mo1->g);
    test($mo5->h == $mo1->h);
    test($mo5->i == $mo1->i);
    test($mo5->j == $mo1->j);
    test($mo5->k == null);
    test($mo5->bs[0] == 5);
    test($mo5->ss == $mo1->ss);
    test($mo5->iid[4] == 3);
    test($mo5->sid["test"] == 10);
    test($mo5->fs == $mo1->fs);
    test($mo5->vs == $mo1->vs);
    test($mo5->shs == $mo1->shs);
    test($mo5->es[0] == $enum && $mo1->es[1] == $enum);
    test($mo5->fss[0] == $fs);
    test($mo5->vss[0] == $vs);
    test($mo5->oos[0]->a == 15);
    test($mo5->oops[0] == $prx);

    test($mo5->ied[4] == $enum);
    test($mo5->ifsd[4] == $fs);
    test($mo5->ivsd[5] == $vs);
    test($mo5->iood[5]->a == 15);
    test($mo5->ioopd[5] == $prx);

    test($mo5->bos == $mo1->bos);

    // Clear the first half of the optional members
    $mo6 = new $mocls;
    $mo6->b = $mo5->b;
    $mo6->d = $mo5->d;
    $mo6->f = $mo5->f;
    $mo6->h = $mo5->h;
    $mo6->j = $mo5->j;
    $mo6->bs = $mo5->bs;
    $mo6->iid = $mo5->iid;
    $mo6->fs = $mo5->fs;
    $mo6->shs = $mo5->shs;
    $mo6->fss = $mo5->fss;
    $mo6->oos = $mo5->oos;
    $mo6->ifsd = $mo5->ifsd;
    $mo6->iood = $mo5->iood;
    $mo6->bos = $mo5->bos;

    $mo7 = $initial->pingPong($mo6);
    test($mo7->a == Ice_Unset);
    test($mo7->b == $mo1->b);
    test($mo7->c == Ice_Unset);
    test($mo7->d == $mo1->d);
    test($mo7->e == Ice_Unset);
    test($mo7->f == $mo1->f);
    test($mo7->g == Ice_Unset);
    test($mo7->h == $mo1->h);
    test($mo7->i == Ice_Unset);
    test($mo7->j == $mo1->j);
    test($mo7->k == Ice_Unset);
    test($mo7->bs[0] == 5);
    test($mo7->ss == Ice_Unset);
    test($mo7->iid[4] == 3);
    test($mo7->sid == Ice_Unset);
    test($mo7->fs == $mo1->fs);
    test($mo7->vs == Ice_Unset);

    test($mo7->shs == $mo1->shs);
    test($mo7->es == Ice_Unset);
    test($mo7->fss[0] == $fs);
    test($mo7->vss == Ice_Unset);
    test($mo7->oos[0]->a == 15);
    test($mo7->oops == Ice_Unset);

    test($mo7->ied == Ice_Unset);
    test($mo7->ifsd[4] == $fs);
    test($mo7->ivsd == Ice_Unset);
    test($mo7->iood[5]->a == 15);
    test($mo7->ioopd == Ice_Unset);

    test($mo7->bos == array(false, true, false));

    // Clear the second half of the optional members
    $mo8 = new $mocls;
    $mo8->a = $mo5->a;
    $mo8->c = $mo5->c;
    $mo8->e = $mo5->e;
    $mo8->g = $mo5->g;
    $mo8->i = $mo5->i;
    $mo8->k = $mo8;
    $mo8->ss = $mo5->ss;
    $mo8->sid = $mo5->sid;
    $mo8->vs = $mo5->vs;

    $mo8->es = $mo5->es;
    $mo8->vss = $mo5->vss;
    $mo8->oops = $mo5->oops;

    $mo8->ied = $mo5->ied;
    $mo8->ivsd = $mo5->ivsd;
    $mo8->ioopd = $mo5->ioopd;

    $mo9 = $initial->pingPong($mo8);
    test($mo9->a == $mo1->a);
    test($mo9->b == Ice_Unset);
    test($mo9->c == $mo1->c);
    test($mo9->d == Ice_Unset);
    test($mo9->e == $mo1->e);
    test($mo9->f == Ice_Unset);
    test($mo9->g == $mo1->g);
    test($mo9->h == Ice_Unset);
    test($mo9->i == $mo1->i);
    test($mo9->j == Ice_Unset);
    test($mo9->k == $mo9);
    test($mo9->bs == Ice_Unset);
    test($mo9->ss == $mo1->ss);
    test($mo9->iid == Ice_Unset);
    test($mo9->sid["test"] == 10);
    test($mo9->fs == Ice_Unset);
    test($mo9->vs == $mo1->vs);

    test($mo9->shs == Ice_Unset);
    test($mo9->es[0] == $enum && $mo1->es[1] == $enum);
    test($mo9->fss == Ice_Unset);
    test($mo9->vss[0] == $vs);
    test($mo9->oos == Ice_Unset);
    test($mo9->oops[0] == $prx);

    test($mo9->ied[4] == $enum);
    test($mo9->ifsd == Ice_Unset);
    test($mo9->ivsd[5] == $vs);
    test($mo9->iood == Ice_Unset);
    test($mo9->ioopd[5] == $prx);

    test($mo9->bos == Ice_Unset);

    //
    // Use the 1.0 encoding with operations whose only class parameters are optional.
    //
    $oo = new $oocls(53);
    $initial->sendOptionalClass(true, $oo);
    $initial->ice_encodingVersion($Ice_Encoding_1_0)->sendOptionalClass(true, $oo);

    $initial->returnOptionalClass(true, $oo);
    test($oo != Ice_Unset);
    $initial->ice_encodingVersion($Ice_Encoding_1_0)->returnOptionalClass(true, $oo);
    test($oo == Ice_Unset);


    $gcls = $NS ? "Test\\G" : "Test_G";
    $g1cls = $NS ? "Test\\G1" : "Test_G1";
    $g2cls = $NS ? "Test\\G2" : "Test_G2";

    $g = new $gcls;
    $g->gg1Opt = new $g1cls("gg1Opt");
    $g->gg2 = new $g2cls(10);
    $g->gg2Opt = new $g2cls(20);
    $g->gg1 = new $g1cls("gg1");
    $r = $initial->opG($g);
    test($r->gg1Opt->a == "gg1Opt");
    test($r->gg2->a == 10);
    test($r->gg2Opt->a == 20);
    test($r->gg1->a == "gg1");

    $initial2 = $NS ? eval("return Test\\Initial2PrxHelper::uncheckedCast(\$base);") :
                      eval("return Test_Initial2PrxHelper::uncheckedCast(\$base);");
    $initial2->opVoid(15, "test");

    echo "ok\n";

    echo "testing marshaling of large containers with fixed size elements... ";
    flush();

    $mc = new $mocls;
    $mc->bs = array();
    for($i = 0; $i < 1000; $i++)
    {
        $mc->bs[$i] = 0;
    }
    $mc->shs = array();
    for($i = 0; $i < 300; $i++)
    {
        $mc->shs[$i] = 0;
    }
    $mc->fss = array();
    for($i = 0; $i < 300; $i++)
    {
        $mc->fss[$i] = new $fscls;
    }
    $mc->ifsd = array();
    for($i = 0; $i < 300; $i++)
    {
        $mc->ifsd[$i] = new $fscls;
    }

    $mc = $initial->pingPong($mc);
    test(count($mc->bs) == 1000);
    test(count($mc->shs) == 300);
    test(count($mc->fss) == 300);
    test(count($mc->ifsd) == 300);

    echo "ok\n";

    echo "testing tag marshaling... ";
    flush();

    $bcls = $NS ? "Test\\B" : "Test_B";
    $b = new $bcls;
    $b2 = $initial->pingPong($b);
    test($b2->ma == Ice_Unset);
    test($b2->mb == Ice_Unset);
    test($b2->mc == Ice_Unset);

    $b->ma = 10;
    $b->mb = 11;
    $b->mc = 12;
    $b->md = 13;

    $b2 = $initial->pingPong($b);
    test($b2->ma == 10);
    test($b2->mb == 11);
    test($b2->mc == 12);
    test($b2->md == 13);

    echo "ok\n";

    echo "testing marshalling of objects with optional objects...";
    flush();

    $fcls = $NS ? "Test\\F" : "Test_F";
    $f = new $fcls;

    $acls = $NS ? "Test\\A" : "Test_A";
    $f->af = new $acls;
    $f->ae = $f->af;

    $rf = $initial->pingPong($f);
    test($rf->ae == $rf->af);

    echo "ok\n";

    echo "testing optional with default values... ";
    flush();

    $wdcls = $NS ? "Test\\WD" : "Test_WD";
    $wd = $initial->pingPong(new $wdcls);
    test($wd->a == 5);
    test($wd->s == "test");
    $wd->a = Ice_Unset;
    $wd->s = Ice_Unset;
    $wd = $initial->pingPong($wd);
    test($wd->a == Ice_Unset);
    test($wd->s == Ice_Unset);

    echo "ok\n";

    if($communicator->getProperties()->getPropertyAsInt("Ice.Default.SlicedFormat") > 0)
    {
        echo "testing marshaling with unknown class slices... ";
        flush();

        $ccls = $NS ? "Test\\C" : "Test_C";
        $c = new $ccls;
        $c->ss = "test";
        $c->ms = "testms";
        $c = $initial->pingPong($c);
        test($c->ma == Ice_Unset);
        test($c->mb == Ice_Unset);
        test($c->mc == Ice_Unset);
        test($c->md == Ice_Unset);
        test($c->ss == "test");
        test($c->ms == "testms");

        echo "ok\n";

        echo "testing optionals with unknown classes... ";
        flush();

        $initial2 = $NS ? eval("return Test\\Initial2PrxHelper::uncheckedCast(\$base);") :
                          eval("return Test_Initial2PrxHelper::uncheckedCast(\$base);");
        $acls = $NS ? "Test\\A" : "Test_A";
        $dcls = $NS ? "Test\\D" : "Test_D";
        $d = new $dcls;
        $d->ds = "test";
        $d->seq = array("test1", "test2", "test3", "test4");
        $d->ao = new $acls(18);
        $d->requiredB = 14;
        $d->requiredA = 14;
        $initial2->opClassAndUnknownOptional(new $acls, $d);

        echo "ok\n";
    }

    echo "testing optional parameters... ";
    flush();

    $p2 = 0;
    $p3 = $initial->opByte(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p3 = $initial->opByte(56, $p2);
    test($p2 == 56 && $p3 == 56);

    $p3 = $initial->opBool(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p3 = $initial->opBool(true, $p2);
    test($p2 == true && $p3 == true);

    $p3 = $initial->opShort(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p3 = $initial->opShort(56, $p2);
    test($p2 == 56 && $p3 == 56);

    $p3 = $initial->opInt(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p3 = $initial->opInt(56, $p2);
    test($p2 == 56 && $p3 == 56);

    $p3 = $initial->opLong(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p3 = $initial->opLong(56, $p2);
    test($p2 == 56 && $p3 == 56);

    $p3 = $initial->opFloat(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p3 = $initial->opFloat(1.0, $p2);
    test($p2 == 1.0 && $p3 == 1.0);

    $p3 = $initial->opDouble(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p3 = $initial->opDouble(1.0, $p2);
    test($p2 == 1.0 && $p3 == 1.0);

    $p3 = $initial->opString(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p3 = $initial->opString("test", $p2);
    test($p2 == "test" && $p3 == "test");

    $p3 = $initial->opMyEnum(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p3 = $initial->opMyEnum($enum, $p2);
    test($p2 == $enum && $p3 == $enum);

    $p3 = $initial->opSmallStruct(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = new $sscls(56);
    $p3 = $initial->opSmallStruct($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);
    $p3 = $initial->opSmallStruct(null, $p2); // Testing null struct
    test($p2->m == 0 && $p3->m == 0);

    $p3 = $initial->opFixedStruct(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = new $fscls(56);
    $p3 = $initial->opFixedStruct($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opVarStruct(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = new $vscls("test");
    $p3 = $initial->opVarStruct($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opOneOptional(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = new $oocls(58);
    $p3 = $initial->opOneOptional($p1, $p2);
    test($p2->a == $p1->a && $p3->a == $p1->a);

    $p3 = $initial->opOneOptionalProxy(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p3 = $initial->opOneOptionalProxy($prx, $p2);
    test($p2 == $prx && $p3 == $prx);

    $p3 = $initial->opByteSeq(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 56;
    }
    $p3 = $initial->opByteSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opBoolSeq(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = true;
    }
    $p3 = $initial->opBoolSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opShortSeq(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 56;
    }
    $p3 = $initial->opShortSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opIntSeq(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 56;
    }
    $p3 = $initial->opIntSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opLongSeq(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 56;
    }
    $p3 = $initial->opLongSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opFloatSeq(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 1.0;
    }
    $p3 = $initial->opFloatSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opDoubleSeq(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 1.0;
    }
    $p3 = $initial->opDoubleSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opStringSeq(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = "test1";
    }
    $p3 = $initial->opStringSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opSmallStructSeq(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = array();
    for($i = 0; $i < 10; $i++)
    {
        $p1[$i] = new $sscls(1);
    }
    $p3 = $initial->opSmallStructSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opFixedStructSeq(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = array();
    for($i = 0; $i < 10; $i++)
    {
        $p1[$i] = new $fscls(1);
    }
    $p3 = $initial->opFixedStructSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opVarStructSeq(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = array();
    for($i = 0; $i < 10; $i++)
    {
        $p1[$i] = new $vscls("test");
    }
    $p3 = $initial->opVarStructSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opIntIntDict(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = array(1=>2, 2=>3);
    $p3 = $initial->opIntIntDict($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opStringIntDict(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = array("1"=>2, "2"=>3);
    $p3 = $initial->opStringIntDict($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opIntOneOptionalDict(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);
    $p1 = array(1=>new $oocls(58), 2=>new $oocls(59));
    $p3 = $initial->opIntOneOptionalDict($p1, $p2);
    test($p2[1]->a == 58 && $p3[1]->a == 58);

    echo "ok\n";

    echo "testing exception optionals... ";
    flush();

    try
    {
        $initial->opOptionalException(Ice_Unset, Ice_Unset, Ice_Unset);
    }
    catch(Exception $ex)
    {
        $excls = $NS ? "Test\\OptionalException" : "Test_OptionalException";
        if(!($ex instanceof $excls))
        {
            throw $ex;
        }
        test($ex->a == Ice_Unset);
        test($ex->b == Ice_Unset);
        test($ex->o == Ice_Unset);
    }

    try
    {
        $initial->opOptionalException(30, "test", new $oocls(53));
    }
    catch(Exception $ex)
    {
        test($ex->a == 30);
        test($ex->b == "test");
        test($ex->o->a == 53);
    }

    try
    {
        //
        // Use the 1.0 encoding with an exception whose only class members are optional.
        //
        $initial->ice_encodingVersion($Ice_Encoding_1_0)->opOptionalException(30, "test", new $oocls(53));
    }
    catch(Exception $ex)
    {
        test($ex->a == Ice_Unset);
        test($ex->b == Ice_Unset);
        test($ex->o == Ice_Unset);
    }

    try
    {
        $initial->opDerivedException(Ice_Unset, Ice_Unset, Ice_Unset);
    }
    catch(Exception $ex)
    {
        test($ex->a == Ice_Unset);
        test($ex->b == Ice_Unset);
        test($ex->o == Ice_Unset);
        test($ex->ss == Ice_Unset);
        test($ex->o2 == Ice_Unset);
    }

    try
    {
        $initial->opDerivedException(30, "test", new $oocls(53));
    }
    catch(Exception $ex)
    {
        test($ex->a == 30);
        test($ex->b == "test");
        test($ex->o->a == 53);
        test($ex->ss == "test");
        test($ex->o2 == $ex->o);
    }

    try
    {
        $initial->opRequiredException(Ice_Unset, Ice_Unset, Ice_Unset);
    }
    catch(Exception $ex)
    {
        test($ex->a == Ice_Unset);
        test($ex->b == Ice_Unset);
        test($ex->o == Ice_Unset);
        test($ex->ss != Ice_Unset);
        test($ex->o2 != Ice_Unset);
    }

    try
    {
        $initial->opRequiredException(30, "test", new $oocls(53));
    }
    catch(Exception $ex)
    {
        test($ex->a == 30);
        test($ex->b == "test");
        test($ex->o->a == 53);
        test($ex->ss == "test");
        test($ex->o2 == $ex->o);
    }

    echo "ok\n";

    echo "testing optionals with marshaled results... ";
    flush();

    test($initial->opMStruct1() != Ice_Unset);
    test($initial->opMDict1() != Ice_Unset);
    test($initial->opMSeq1() != Ice_Unset);
    test($initial->opMG1() != Ice_Unset);

    $p3 = $initial->opMStruct2(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);

    $sscls = $NS ? "Test\\SmallStruct" : "Test_SmallStruct";
    $p1 = new $sscls(56);
    $p3 = $initial->opMStruct2($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opMSeq2(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);

    $p1 = array("hello");
    $p3 = $initial->opMSeq2($p1, $p2);
    test($p2[0] == "hello" && $p3[0] == "hello");

    $p3 = $initial->opMDict2(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);

    $p1 = array("test" => 54);
    $p3 = $initial->opMDict2($p1, $p2);
    test($p2["test"] == 54 && $p3["test"] == 54);

    $p3 = $initial->opMG2(Ice_Unset, $p2);
    test($p2 == Ice_Unset && $p3 == Ice_Unset);

    $p1 = new $gcls;
    $p3 = $initial->opMG2($p1, $p2);
    test($p2 != Ice_Unset && $p3 != Ice_Unset && $p3 == $p2);

    echo "ok\n";

    return $initial;
}

$communicator = $NS ? eval("return Ice\\initialize(\$argv);") : 
                      eval("return Ice_initialize(\$argv);");

$initial = allTests($communicator);

$initial->shutdown();
$communicator->destroy();

exit();
?>
