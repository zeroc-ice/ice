<?php
// Copyright (c) ZeroC, Inc.

require_once('ClientPrivate.php');

function allTests($helper)
{
    global $Ice_Encoding_1_0;

    $ref = sprintf("initial:%s", $helper->getTestEndpoint());
    $communicator = $helper->communicator();
    $initial = Test\InitialPrxHelper::createProxy($communicator, $ref);

    echo "testing optional data members... ";
    flush();

    $oo1 = new Test\OneOptional;
    test($oo1->a == Ice\None);
    $oo1->a = 15;

    $oo2 = new Test\OneOptional(16);
    test($oo2->a == 16);

    $mo1 = new Test\MultiOptional;
    test($mo1->a == Ice\None);
    test($mo1->b == Ice\None);
    test($mo1->c == Ice\None);
    test($mo1->d == Ice\None);
    test($mo1->e == Ice\None);
    test($mo1->f == Ice\None);
    test($mo1->g == Ice\None);
    test($mo1->h == Ice\None);
    test($mo1->i == Ice\None);
    test($mo1->j == Ice\None);
    test($mo1->bs == Ice\None);
    test($mo1->ss == Ice\None);
    test($mo1->iid == Ice\None);
    test($mo1->sid == Ice\None);
    test($mo1->fs == Ice\None);
    test($mo1->vs == Ice\None);

    test($mo1->shs == Ice\None);
    test($mo1->es == Ice\None);
    test($mo1->fss == Ice\None);
    test($mo1->vss == Ice\None);
    test($mo1->mips == Ice\None);

    test($mo1->ied == Ice\None);
    test($mo1->ifsd == Ice\None);
    test($mo1->ivsd == Ice\None);
    test($mo1->imipd == Ice\None);

    test($mo1->bos == Ice\None);

    $ss = new Test\SmallStruct();
    $fs = new Test\FixedStruct(78);
    $vs = new Test\VarStruct("hello");
    $prx = Test\MyInterfacePrxHelper::createProxy($communicator, "test");
    $mo1 = new Test\MultiOptional(15, true, 19, 78, 99, 5.5, 1.0, 'test', Test\MyEnum::MyEnumMember,
                      $prx, array(5), array('test', 'test2'), array(4=>3), array('test'=>10),
                      $fs, $vs, array(1), array(Test\MyEnum::MyEnumMember, Test\MyEnum::MyEnumMember), array($fs), array($vs),
                      array($prx), array(4=>Test\MyEnum::MyEnumMember), array(4=>$fs), array(5=>$vs),
                      array(5=>$prx), array(false, true, false));

    test($mo1->a == 15);
    test($mo1->b == true);
    test($mo1->c == 19);
    test($mo1->d == 78);
    test($mo1->e == 99);
    test($mo1->f == 5.5);
    test($mo1->g == 1.0);
    test($mo1->h == "test");
    test($mo1->i == Test\MyEnum::MyEnumMember);
    test($mo1->j == $prx);
    test($mo1->bs == array(5));
    test($mo1->ss == array("test", "test2"));
    test($mo1->iid[4] == 3);
    test($mo1->sid["test"] == 10);
    test($mo1->fs == $fs);
    test($mo1->vs == $vs);

    test($mo1->shs[0] == 1);
    test($mo1->es[0] == Test\MyEnum::MyEnumMember && $mo1->es[1] == Test\MyEnum::MyEnumMember);
    test($mo1->fss[0] == $fs);
    test($mo1->vss[0] == $vs);
    test($mo1->mips[0] == $prx);

    test($mo1->ied[4] == Test\MyEnum::MyEnumMember);
    test($mo1->ifsd[4] == $fs);
    test($mo1->ivsd[5] == $vs);
    test($mo1->imipd[5] == $prx);

    test($mo1->bos == array(false, true, false));

    //
    // Test generated struct and classes compare with Ice\None
    //
    test($ss != Ice\None);
    test($fs != Ice\None);
    test($vs != Ice\None);
    test($mo1 != Ice\None);

    echo "ok\n";

    echo "testing marshaling... ";
    flush();

    $oo4 = $initial->pingPong(new Test\OneOptional);
    test($oo4->a == Ice\None);

    $oo5 = $initial->pingPong($oo1);
    test($oo1->a == $oo5->a);

    $mo4 = $initial->pingPong(new Test\MultiOptional);
    test($mo4->a == Ice\None);
    test($mo4->b == Ice\None);
    test($mo4->c == Ice\None);
    test($mo4->d == Ice\None);
    test($mo4->e == Ice\None);
    test($mo4->f == Ice\None);
    test($mo4->g == Ice\None);
    test($mo4->h == Ice\None);
    test($mo4->i == Ice\None);
    test($mo4->j == Ice\None);
    test($mo4->bs == Ice\None);
    test($mo4->ss == Ice\None);
    test($mo4->iid == Ice\None);
    test($mo4->sid == Ice\None);
    test($mo4->fs == Ice\None);
    test($mo4->vs == Ice\None);

    test($mo4->shs == Ice\None);
    test($mo4->es == Ice\None);
    test($mo4->fss == Ice\None);
    test($mo4->vss == Ice\None);
    test($mo4->mips == Ice\None);

    test($mo4->ied == Ice\None);
    test($mo4->ifsd == Ice\None);
    test($mo4->ivsd == Ice\None);
    test($mo4->imipd == Ice\None);

    test($mo4->bos == Ice\None);

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
    test($mo5->bs[0] == 5);
    test($mo5->ss == $mo1->ss);
    test($mo5->iid[4] == 3);
    test($mo5->sid["test"] == 10);
    test($mo5->fs == $mo1->fs);
    test($mo5->vs == $mo1->vs);
    test($mo5->shs == $mo1->shs);
    test($mo5->es[0] == Test\MyEnum::MyEnumMember && $mo1->es[1] == Test\MyEnum::MyEnumMember);
    test($mo5->fss[0] == $fs);
    test($mo5->vss[0] == $vs);
    test($mo5->mips[0] == $prx);

    test($mo5->ied[4] == Test\MyEnum::MyEnumMember);
    test($mo5->ifsd[4] == $fs);
    test($mo5->ivsd[5] == $vs);
    test($mo5->imipd[5] == $prx);

    test($mo5->bos == $mo1->bos);

    // Clear the first half of the optional members
    $mo6 = new Test\MultiOptional;
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
    $mo6->ifsd = $mo5->ifsd;
    $mo6->bos = $mo5->bos;

    $mo7 = $initial->pingPong($mo6);
    test($mo7->a == Ice\None);
    test($mo7->b == $mo1->b);
    test($mo7->c == Ice\None);
    test($mo7->d == $mo1->d);
    test($mo7->e == Ice\None);
    test($mo7->f == $mo1->f);
    test($mo7->g == Ice\None);
    test($mo7->h == $mo1->h);
    test($mo7->i == Ice\None);
    test($mo7->j == $mo1->j);
    test($mo7->bs[0] == 5);
    test($mo7->ss == Ice\None);
    test($mo7->iid[4] == 3);
    test($mo7->sid == Ice\None);
    test($mo7->fs == $mo1->fs);
    test($mo7->vs == Ice\None);

    test($mo7->shs == $mo1->shs);
    test($mo7->es == Ice\None);
    test($mo7->fss[0] == $fs);
    test($mo7->vss == Ice\None);
    test($mo7->mips == Ice\None);

    test($mo7->ied == Ice\None);
    test($mo7->ifsd[4] == $fs);
    test($mo7->ivsd == Ice\None);
    test($mo7->imipd == Ice\None);

    test($mo7->bos == array(false, true, false));

    // Clear the second half of the optional members
    $mo8 = new Test\MultiOptional;
    $mo8->a = $mo5->a;
    $mo8->c = $mo5->c;
    $mo8->e = $mo5->e;
    $mo8->g = $mo5->g;
    $mo8->i = $mo5->i;
    $mo8->ss = $mo5->ss;
    $mo8->sid = $mo5->sid;
    $mo8->vs = $mo5->vs;

    $mo8->es = $mo5->es;
    $mo8->vss = $mo5->vss;
    $mo8->mips = $mo5->mips;

    $mo8->ied = $mo5->ied;
    $mo8->ivsd = $mo5->ivsd;
    $mo8->imipd = $mo5->imipd;

    $mo9 = $initial->pingPong($mo8);
    test($mo9->a == $mo1->a);
    test($mo9->b == Ice\None);
    test($mo9->c == $mo1->c);
    test($mo9->d == Ice\None);
    test($mo9->e == $mo1->e);
    test($mo9->f == Ice\None);
    test($mo9->g == $mo1->g);
    test($mo9->h == Ice\None);
    test($mo9->i == $mo1->i);
    test($mo9->j == Ice\None);
    test($mo9->bs == Ice\None);
    test($mo9->ss == $mo1->ss);
    test($mo9->iid == Ice\None);
    test($mo9->sid["test"] == 10);
    test($mo9->fs == Ice\None);
    test($mo9->vs == $mo1->vs);

    test($mo9->shs == Ice\None);
    test($mo9->es[0] == Test\MyEnum::MyEnumMember && $mo1->es[1] == Test\MyEnum::MyEnumMember);
    test($mo9->fss == Ice\None);
    test($mo9->vss[0] == $vs);
    test($mo9->mips[0] == $prx);

    test($mo9->ied[4] == Test\MyEnum::MyEnumMember);
    test($mo9->ifsd == Ice\None);
    test($mo9->ivsd[5] == $vs);
    test($mo9->imipd[5] == $prx);

    test($mo9->bos == Ice\None);

    $g = new Test\G;
    $g->gg1Opt = new Test\G1("gg1Opt");
    $g->gg2 = new Test\G2(10);
    $g->gg2Opt = new Test\G2(20);
    $g->gg1 = new Test\G1("gg1");
    $r = $initial->opG($g);
    test($r->gg1Opt->a == "gg1Opt");
    test($r->gg2->a == 10);
    test($r->gg2Opt->a == 20);
    test($r->gg1->a == "gg1");

    $initial2 = Test\Initial2PrxHelper::uncheckedCast($initial);
    $initial2->opVoid(15, "test");

    echo "ok\n";

    echo "testing marshaling of large containers with fixed size elements... ";
    flush();

    $mc = new Test\MultiOptional;
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
        $mc->fss[$i] = new Test\FixedStruct;
    }
    $mc->ifsd = array();
    for($i = 0; $i < 300; $i++)
    {
        $mc->ifsd[$i] = new Test\FixedStruct;
    }

    $mc = $initial->pingPong($mc);
    test(count($mc->bs) == 1000);
    test(count($mc->shs) == 300);
    test(count($mc->fss) == 300);
    test(count($mc->ifsd) == 300);

    echo "ok\n";

    echo "testing tag marshaling... ";
    flush();

    $b = new Test\B;
    $b2 = $initial->pingPong($b);
    test($b2->ma == Ice\None);
    test($b2->mb == Ice\None);
    test($b2->mc == Ice\None);

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

    echo "testing marshaling of objects with optional members...";
    flush();

    $f = new Test\F;

    $f->fsf = new Test\FixedStruct;
    $f->fse = $f->fsf;

    $rf = $initial->pingPong($f);
    test($rf->fse == $rf->fsf);

    echo "ok\n";

    echo "testing optional with default values... ";
    flush();

    $wd = $initial->pingPong(new Test\WD);
    test($wd->a == 5);
    test($wd->s == "test");
    $wd->a = Ice\None;
    $wd->s = Ice\None;
    $wd = $initial->pingPong($wd);
    test($wd->a == Ice\None);
    test($wd->s == Ice\None);

    echo "ok\n";

    if($communicator->getProperties()->getIcePropertyAsInt("Ice.Default.SlicedFormat") > 0)
    {
        echo "testing marshaling with unknown class slices... ";
        flush();

        $c = new Test\C;
        $c->ss = "test";
        $c->ms = "testms";
        $c = $initial->pingPong($c);
        test($c->ma == Ice\None);
        test($c->mb == Ice\None);
        test($c->mc == Ice\None);
        test($c->md == Ice\None);
        test($c->ss == "test");
        test($c->ms == "testms");

        echo "ok\n";

        echo "testing operations with unknown optionals... ";
        flush();

        $initial2 = Test\Initial2PrxHelper::uncheckedCast($initial);
        $ovs = new Test\VarStruct("test");
        $initial2->opClassAndUnknownOptional(new Test\A, $ovs);

        echo "ok\n";
    }

    echo "testing optional parameters... ";
    flush();

    $p2 = 0;
    $p3 = $initial->opByte(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p3 = $initial->opByte(56, $p2);
    test($p2 == 56 && $p3 == 56);

    $p3 = $initial->opBool(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p3 = $initial->opBool(true, $p2);
    test($p2 == true && $p3 == true);

    $p3 = $initial->opShort(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p3 = $initial->opShort(56, $p2);
    test($p2 == 56 && $p3 == 56);

    $p3 = $initial->opInt(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p3 = $initial->opInt(56, $p2);
    test($p2 == 56 && $p3 == 56);

    $p3 = $initial->opLong(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p3 = $initial->opLong(56, $p2);
    test($p2 == 56 && $p3 == 56);

    $p3 = $initial->opFloat(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p3 = $initial->opFloat(1.0, $p2);
    test($p2 == 1.0 && $p3 == 1.0);

    $p3 = $initial->opDouble(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p3 = $initial->opDouble(1.0, $p2);
    test($p2 == 1.0 && $p3 == 1.0);

    $p3 = $initial->opString(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p3 = $initial->opString("test", $p2);
    test($p2 == "test" && $p3 == "test");

    $p3 = $initial->opMyEnum(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p3 = $initial->opMyEnum(Test\MyEnum::MyEnumMember, $p2);
    test($p2 == Test\MyEnum::MyEnumMember && $p3 == Test\MyEnum::MyEnumMember);

    $p3 = $initial->opSmallStruct(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p1 = new Test\SmallStruct(56);
    $p3 = $initial->opSmallStruct($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);
    $p3 = $initial->opSmallStruct(null, $p2); // Testing null struct
    test($p2->m == 0 && $p3->m == 0);

    $p3 = $initial->opFixedStruct(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p1 = new Test\FixedStruct(56);
    $p3 = $initial->opFixedStruct($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opVarStruct(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p1 = new Test\VarStruct("test");
    $p3 = $initial->opVarStruct($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p1 = new Test\OneOptional;
    $p3 = $initial->opOneOptional($p1, $p2);
    test($p2->a == Ice\None && $p3->a == Ice\None);
    $p1 = new Test\OneOptional(58);
    $p3 = $initial->opOneOptional($p1, $p2);
    test($p2->a == $p1->a && $p3->a == $p1->a);

    $p3 = $initial->opMyInterfaceProxy(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p3 = $initial->opMyInterfaceProxy($prx, $p2);
    test($p2 == $prx && $p3 == $prx);

    $p3 = $initial->opByteSeq(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 56;
    }
    $p3 = $initial->opByteSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opBoolSeq(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = true;
    }
    $p3 = $initial->opBoolSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opShortSeq(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 56;
    }
    $p3 = $initial->opShortSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opIntSeq(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 56;
    }
    $p3 = $initial->opIntSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opLongSeq(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 56;
    }
    $p3 = $initial->opLongSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opFloatSeq(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 1.0;
    }
    $p3 = $initial->opFloatSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opDoubleSeq(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = 1.0;
    }
    $p3 = $initial->opDoubleSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opStringSeq(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p1 = array();
    for($i = 0; $i < 100; $i++)
    {
        $p1[$i] = "test1";
    }
    $p3 = $initial->opStringSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opSmallStructSeq(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p1 = array();
    for($i = 0; $i < 10; $i++)
    {
        $p1[$i] = new Test\SmallStruct(1);
    }
    $p3 = $initial->opSmallStructSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opFixedStructSeq(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p1 = array();
    for($i = 0; $i < 10; $i++)
    {
        $p1[$i] = new Test\FixedStruct(1);
    }
    $p3 = $initial->opFixedStructSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opVarStructSeq(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p1 = array();
    for($i = 0; $i < 10; $i++)
    {
        $p1[$i] = new Test\VarStruct("test");
    }
    $p3 = $initial->opVarStructSeq($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opIntIntDict(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p1 = array(1=>2, 2=>3);
    $p3 = $initial->opIntIntDict($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opStringIntDict(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);
    $p1 = array("1"=>2, "2"=>3);
    $p3 = $initial->opStringIntDict($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    echo "ok\n";

    echo "testing exception optionals... ";
    flush();

    try
    {
        $initial->opOptionalException(Ice\None, Ice\None);
    }
    catch(Test\OptionalException $ex)
    {
        test($ex->a == Ice\None);
        test($ex->b == Ice\None);
    }

    try
    {
        $initial->opOptionalException(30, "test");
    }
    catch(Exception $ex)
    {
        test($ex->a == 30);
        test($ex->b == "test");
    }

    try
    {
        //
        // Use the 1.0 encoding with an exception whose only data members are optional.
        //
        $initial->ice_encodingVersion($Ice_Encoding_1_0)->opOptionalException(30, "test");
    }
    catch(Exception $ex)
    {
        test($ex->a == Ice\None);
        test($ex->b == Ice\None);
    }

    try
    {
        $initial->opDerivedException(Ice\None, Ice\None);
    }
    catch(Exception $ex)
    {
        test($ex->a == Ice\None);
        test($ex->b == Ice\None);
        test($ex->ss == Ice\None);
        test($ex->d1 == "d1");
        test($ex->d2 == "d2");
    }

    try
    {
        $initial->opDerivedException(30, "test");
    }
    catch(Exception $ex)
    {
        test($ex->a == 30);
        test($ex->b == "test");
        test($ex->ss == "test");
        test($ex->d1 == "d1");
        test($ex->d2 == "d2");
    }

    try
    {
        $initial->opRequiredException(Ice\None, Ice\None);
    }
    catch(Exception $ex)
    {
        test($ex->a == Ice\None);
        test($ex->b == Ice\None);
        test($ex->ss != Ice\None);
    }

    try
    {
        $initial->opRequiredException(30, "test");
    }
    catch(Exception $ex)
    {
        test($ex->a == 30);
        test($ex->b == "test");
        test($ex->ss == "test");
    }

    echo "ok\n";

    echo "testing optionals with marshaled results... ";
    flush();

    test($initial->opMStruct1() != Ice\None);
    test($initial->opMDict1() != Ice\None);
    test($initial->opMSeq1() != Ice\None);

    $p3 = $initial->opMStruct2(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);

    $p1 = new Test\SmallStruct(56);
    $p3 = $initial->opMStruct2($p1, $p2);
    test($p2 == $p1 && $p3 == $p1);

    $p3 = $initial->opMSeq2(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);

    $p1 = array("hello");
    $p3 = $initial->opMSeq2($p1, $p2);
    test($p2[0] == "hello" && $p3[0] == "hello");

    $p3 = $initial->opMDict2(Ice\None, $p2);
    test($p2 == Ice\None && $p3 == Ice\None);

    $p1 = array("test" => 54);
    $p3 = $initial->opMDict2($p1, $p2);
    test($p2["test"] == 54 && $p3["test"] == 54);

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
