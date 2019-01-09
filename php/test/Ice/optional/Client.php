<?php
// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

require_once('ClientPrivate.php');

function allTests($helper)
{
    global $NS;
    global $Ice_Encoding_1_0;

    $enum = $NS ? constant("Test\\MyEnum::MyEnumMember") : constant("Test_MyEnum::MyEnumMember");
    $none = $NS ? constant("Ice\\None") : constant("Ice_Unset");

    echo "testing stringToProxy... ";
    flush();
    $ref = sprintf("initial:%s", $helper->getTestEndpoint());
    $communicator = $helper->communicator();
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
    test($oo1->a == $none);
    $oo1->a = 15;

    $oo2 = new $oocls(16);
    test($oo2->a == 16);

    $mocls = $NS ? "Test\\MultiOptional" : "Test_MultiOptional";
    $mo1 = new $mocls;
    test($mo1->a == $none);
    test($mo1->b == $none);
    test($mo1->c == $none);
    test($mo1->d == $none);
    test($mo1->e == $none);
    test($mo1->f == $none);
    test($mo1->g == $none);
    test($mo1->h == $none);
    test($mo1->i == $none);
    test($mo1->j == $none);
    test($mo1->k == $none);
    test($mo1->bs == $none);
    test($mo1->ss == $none);
    test($mo1->iid == $none);
    test($mo1->sid == $none);
    test($mo1->fs == $none);
    test($mo1->vs == $none);

    test($mo1->shs == $none);
    test($mo1->es == $none);
    test($mo1->fss == $none);
    test($mo1->vss == $none);
    test($mo1->oos == $none);
    test($mo1->oops == $none);

    test($mo1->ied == $none);
    test($mo1->ifsd == $none);
    test($mo1->ivsd == $none);
    test($mo1->iood == $none);
    test($mo1->ioopd == $none);

    test($mo1->bos == $none);

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
    // Test generated struct and classes compare with $none
    //
    test($ss != $none);
    test($fs != $none);
    test($vs != $none);
    test($mo1 != $none);

    echo "ok\n";

    echo "testing marshaling... ";
    flush();

    $oo4 = $initial->pingPong(new $oocls);
    test($oo4->a == $none);

    $oo5 = $initial->pingPong($oo1);
    test($oo1->a == $oo5->a);

    $mo4 = $initial->pingPong(new $mocls);
    test($mo4->a == $none);
    test($mo4->b == $none);
    test($mo4->c == $none);
    test($mo4->d == $none);
    test($mo4->e == $none);
    test($mo4->f == $none);
    test($mo4->g == $none);
    test($mo4->h == $none);
    test($mo4->i == $none);
    test($mo4->j == $none);
    test($mo4->k == $none);
    test($mo4->bs == $none);
    test($mo4->ss == $none);
    test($mo4->iid == $none);
    test($mo4->sid == $none);
    test($mo4->fs == $none);
    test($mo4->vs == $none);

    test($mo4->shs == $none);
    test($mo4->es == $none);
    test($mo4->fss == $none);
    test($mo4->vss == $none);
    test($mo4->oos == $none);
    test($mo4->oops == $none);

    test($mo4->ied == $none);
    test($mo4->ifsd == $none);
    test($mo4->ivsd == $none);
    test($mo4->iood == $none);
    test($mo4->ioopd == $none);

    test($mo4->bos == $none);

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
    test($mo7->a == $none);
    test($mo7->b == $mo1->b);
    test($mo7->c == $none);
    test($mo7->d == $mo1->d);
    test($mo7->e == $none);
    test($mo7->f == $mo1->f);
    test($mo7->g == $none);
    test($mo7->h == $mo1->h);
    test($mo7->i == $none);
    test($mo7->j == $mo1->j);
    test($mo7->k == $none);
    test($mo7->bs[0] == 5);
    test($mo7->ss == $none);
    test($mo7->iid[4] == 3);
    test($mo7->sid == $none);
    test($mo7->fs == $mo1->fs);
    test($mo7->vs == $none);

    test($mo7->shs == $mo1->shs);
    test($mo7->es == $none);
    test($mo7->fss[0] == $fs);
    test($mo7->vss == $none);
    test($mo7->oos[0]->a == 15);
    test($mo7->oops == $none);

    test($mo7->ied == $none);
    test($mo7->ifsd[4] == $fs);
    test($mo7->ivsd == $none);
    test($mo7->iood[5]->a == 15);
    test($mo7->ioopd == $none);

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
    test($mo9->b == $none);
    test($mo9->c == $mo1->c);
    test($mo9->d == $none);
    test($mo9->e == $mo1->e);
    test($mo9->f == $none);
    test($mo9->g == $mo1->g);
    test($mo9->h == $none);
    test($mo9->i == $mo1->i);
    test($mo9->j == $none);
    test($mo9->k == $mo9);
    test($mo9->bs == $none);
    test($mo9->ss == $mo1->ss);
    test($mo9->iid == $none);
    test($mo9->sid["test"] == 10);
    test($mo9->fs == $none);
    test($mo9->vs == $mo1->vs);

    test($mo9->shs == $none);
    test($mo9->es[0] == $enum && $mo1->es[1] == $enum);
    test($mo9->fss == $none);
    test($mo9->vss[0] == $vs);
    test($mo9->oos == $none);
    test($mo9->oops[0] == $prx);

    test($mo9->ied[4] == $enum);
    test($mo9->ifsd == $none);
    test($mo9->ivsd[5] == $vs);
    test($mo9->iood == $none);
    test($mo9->ioopd[5] == $prx);

    test($mo9->bos == $none);

    //
    // Use the 1.0 encoding with operations whose only class parameters are optional.
    //
    $oo = new $oocls(53);
    $initial->sendOptionalClass(true, $oo);
    $initial->ice_encodingVersion($Ice_Encoding_1_0)->sendOptionalClass(true, $oo);

    $initial->returnOptionalClass(true, $oo);
    test($oo != $none);
    $initial->ice_encodingVersion($Ice_Encoding_1_0)->returnOptionalClass(true, $oo);
    test($oo == $none);

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
    test($b2->ma == $none);
    test($b2->mb == $none);
    test($b2->mc == $none);

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
    $wd->a = $none;
    $wd->s = $none;
    $wd = $initial->pingPong($wd);
    test($wd->a == $none);
    test($wd->s == $none);

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
        test($c->ma == $none);
        test($c->mb == $none);
        test($c->mc == $none);
        test($c->md == $none);
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
    $p3 = $initial->opByte($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p3 = $initial->opByte(56, $p2);
    test($p2 == 56 && $p3 == 56);

    $p3 = $initial->opBool($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p3 = $initial->opBool(true, $p2);
    test($p2 == true && $p3 == true);

    $p3 = $initial->opShort($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p3 = $initial->opShort(56, $p2);
    test($p2 == 56 && $p3 == 56);

    $p3 = $initial->opInt($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p3 = $initial->opInt(56, $p2);
    test($p2 == 56 && $p3 == 56);

    $p3 = $initial->opLong($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p3 = $initial->opLong(56, $p2);
    test($p2 == 56 && $p3 == 56);

    $p3 = $initial->opFloat($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p3 = $initial->opFloat(1.0, $p2);
    test($p2 == 1.0 && $p3 == 1.0);

    $p3 = $initial->opDouble($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p3 = $initial->opDouble(1.0, $p2);
    test($p2 == 1.0 && $p3 == 1.0);

    $p3 = $initial->opString($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p3 = $initial->opString("test", $p2);
    test($p2 == "test" && $p3 == "test");

    $p3 = $initial->opMyEnum($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p3 = $initial->opMyEnum($enum, $p2);
    test($p2 == $enum && $p3 == $enum);

    $p3 = $initial->opSmallStruct($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p1 = new $sscls(56);
    $p3 = $initial->opSmallStruct($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);
    $p3 = $initial->opSmallStruct(null, $p2); // Testing null struct
    test($p2->m == 0 && $p3->m == 0);

    $p3 = $initial->opFixedStruct($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p1 = new $fscls(56);
    $p3 = $initial->opFixedStruct($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opVarStruct($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p1 = new $vscls("test");
    $p3 = $initial->opVarStruct($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opOneOptional($none, $p2);
    test($p2 == $none && $p3 == $none);
    if($initial->supportsNullOptional())
    {
        $p3 = $initial->opOneOptional(null, $p2);
        test($p2 == null && $p3 == null);
    }
    $p1 = new $oocls(58);
    $p3 = $initial->opOneOptional($p1, $p2);
    test($p2->a == $p1->a && $p3->a == $p1->a);

    $p3 = $initial->opOneOptionalProxy($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p3 = $initial->opOneOptionalProxy($prx, $p2);
    test($p2 == $prx && $p3 == $prx);

    $p3 = $initial->opByteSeq($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 56;
    }
    $p3 = $initial->opByteSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opBoolSeq($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = true;
    }
    $p3 = $initial->opBoolSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opShortSeq($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 56;
    }
    $p3 = $initial->opShortSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opIntSeq($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 56;
    }
    $p3 = $initial->opIntSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opLongSeq($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 56;
    }
    $p3 = $initial->opLongSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opFloatSeq($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 1.0;
    }
    $p3 = $initial->opFloatSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opDoubleSeq($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 1.0;
    }
    $p3 = $initial->opDoubleSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opStringSeq($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = "test1";
    }
    $p3 = $initial->opStringSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opSmallStructSeq($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p1 = array();
    for($i = 0; $i < 10; $i++)
    {
        $p1[$i] = new $sscls(1);
    }
    $p3 = $initial->opSmallStructSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opFixedStructSeq($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p1 = array();
    for($i = 0; $i < 10; $i++)
    {
        $p1[$i] = new $fscls(1);
    }
    $p3 = $initial->opFixedStructSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opVarStructSeq($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p1 = array();
    for($i = 0; $i < 10; $i++)
    {
        $p1[$i] = new $vscls("test");
    }
    $p3 = $initial->opVarStructSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opIntIntDict($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p1 = array(1=>2, 2=>3);
    $p3 = $initial->opIntIntDict($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opStringIntDict($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p1 = array("1"=>2, "2"=>3);
    $p3 = $initial->opStringIntDict($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opIntOneOptionalDict($none, $p2);
    test($p2 == $none && $p3 == $none);
    $p1 = array(1=>new $oocls(58), 2=>new $oocls(59));
    $p3 = $initial->opIntOneOptionalDict($p1, $p2);
    test($p2[1]->a == 58 && $p3[1]->a == 58);

    echo "ok\n";

    echo "testing exception optionals... ";
    flush();

    try
    {
        $initial->opOptionalException($none, $none, $none);
    }
    catch(Exception $ex)
    {
        $excls = $NS ? "Test\\OptionalException" : "Test_OptionalException";
        if(!($ex instanceof $excls))
        {
            throw $ex;
        }
        test($ex->a == $none);
        test($ex->b == $none);
        test($ex->o == $none);
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
        test($ex->a == $none);
        test($ex->b == $none);
        test($ex->o == $none);
    }

    try
    {
        $initial->opDerivedException($none, $none, $none);
    }
    catch(Exception $ex)
    {
        test($ex->a == $none);
        test($ex->b == $none);
        test($ex->o == $none);
        test($ex->ss == $none);
        test($ex->o2 == $none);
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
        $initial->opRequiredException($none, $none, $none);
    }
    catch(Exception $ex)
    {
        test($ex->a == $none);
        test($ex->b == $none);
        test($ex->o == $none);
        test($ex->ss != $none);
        test($ex->o2 != $none);
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

    test($initial->opMStruct1() != $none);
    test($initial->opMDict1() != $none);
    test($initial->opMSeq1() != $none);
    test($initial->opMG1() != $none);

    $p3 = $initial->opMStruct2($none, $p2);
    test($p2 == $none && $p3 == $none);

    $sscls = $NS ? "Test\\SmallStruct" : "Test_SmallStruct";
    $p1 = new $sscls(56);
    $p3 = $initial->opMStruct2($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opMSeq2($none, $p2);
    test($p2 == $none && $p3 == $none);

    $p1 = array("hello");
    $p3 = $initial->opMSeq2($p1, $p2);
    test($p2[0] == "hello" && $p3[0] == "hello");

    $p3 = $initial->opMDict2($none, $p2);
    test($p2 == $none && $p3 == $none);

    $p1 = array("test" => 54);
    $p3 = $initial->opMDict2($p1, $p2);
    test($p2["test"] == 54 && $p3["test"] == 54);

    $p3 = $initial->opMG2($none, $p2);
    test($p2 == $none && $p3 == $none);

    $p1 = new $gcls;
    $p3 = $initial->opMG2($p1, $p2);
    test($p2 != $none && $p3 != $none && $p3 == $p2);

    echo "ok\n";

    return $initial;
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
?>
