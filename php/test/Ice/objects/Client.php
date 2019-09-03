<?php
//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

$NS = function_exists("Ice\\initialize");
require_once('Test.php');
require_once('Forward.php');

if($NS)
{
    $code = <<<EOT
        class Test_A1 extends Test\A1 {}
        abstract class Test_B extends Test\B {}
        abstract class Test_C extends Test\C {}
        abstract class Test_D extends Test\D {}
        class Test_D1 extends Test\D1 {}
        abstract class Test_E extends Test\E {}
        abstract class Test_F extends Test\F {}
        class Test_G extends Test\G {}
        class Test_H extends Test\H {}
        class Test_Recursive extends Test\Recursive {}
        class Ice_Value extends Ice\Value {}
        class Ice_InterfaceByValue extends Ice\InterfaceByValue {}
        interface Ice_ObjectFactory extends Ice\ObjectFactory {}
        interface Ice_ValueFactory extends Ice\ValueFactory {}
        class Test_L extends Test\L {}
        class Test_F1 extends Test\F1 {}
        class Test_F3 extends Test\F3 {}
EOT;
    eval($code);
}

class BI extends Test_B
{
    function ice_pretUnmarshal()
    {
        $this->preUnmarshalInvoked = true;
    }

    function ice_postUnmarshal()
    {
        $this->postUnmarshalInvoked = true;
    }
}

class CI extends Test_C
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

class DI extends Test_D
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

class EI extends Test_E
{
    function __construct()
    {
        $this->i = 1;
        $this->s = "hello";
    }

    function checkValues()
    {
        return $this->i == 1 && $this->s == "hello";
    }
}

class FI extends Test_F
{
    function __construct($e=null)
    {
        $this->e1 = $e;
        $this->e2 = $e;
    }

    function checkValues()
    {
        return $this->e1 != null && $this->e1 === $this->e2;
    }
}

class II extends Ice_InterfaceByValue
{
    public function __construct()
    {
        parent::__construct("::Test::I");
    }
}

class JI extends Ice_InterfaceByValue
{
    public function __construct()
    {
        parent::__construct("::Test::J");
    }
}

class HI extends Test_H
{
}

class MyValueFactory implements Ice_ValueFactory
{
    function create($id)
    {
        if($id == "::Test::B")
        {
            return new BI();
        }
        else if($id == "::Test::C")
        {
            return new CI();
        }
        else if($id == "::Test::D")
        {
            return new DI();
        }
        else if($id == "::Test::E")
        {
            return new EI();
        }
        else if($id == "::Test::F")
        {
            return new FI();
        }
        else if($id == "::Test::I")
        {
            return new II();
        }
        else if($id == "::Test::J")
        {
            return new JI();
        }
        else if($id == "::Test::H")
        {
            return new HI();
        }
        return null;
    }
}

class MyObjectFactory implements Ice_ObjectFactory
{
    function create($id)
    {
        return null;
    }

    function destroy()
    {
        // Do nothing
    }
}

function allTests($helper)
{
    global $NS;

    echo "testing stringToProxy... ";
    flush();
    $ref = sprintf("initial:%s", $helper->getTestEndpoint());
    $communicator = $helper->communicator();
    $base = $communicator->stringToProxy($ref);
    test($base != null);
    echo "ok\n";

    echo "testing checked cast... ";
    flush();
    $initial = $base->ice_checkedCast("::Test::Initial");
    test($initial != null);
    test($initial == $base);
    echo "ok\n";

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

    echo "testing protected members... ";
    flush();
    $e = $initial->getE();
    test($e->checkValues());
    $prop = new ReflectionProperty("Test_E", "i");
    test($prop->isProtected());
    $prop = new ReflectionProperty("Test_E", "s");
    test($prop->isProtected());
    $f = $initial->getF();
    test($f->checkValues());
    test($f->e2->checkValues());
    $prop = new ReflectionProperty("Test_F", "e1");
    test($prop->isProtected());
    $prop = new ReflectionProperty("Test_F", "e2");
    test($prop->isPublic());
    echo "ok\n";

    echo "getting I, J and H... ";
    flush();
    $i = $initial->getI();
    test($i != null);
    $j = $initial->getJ();
    test($j != null and $j instanceof JI);
    $h = $initial->getH();
    test($h != null and $h instanceof HI);
    echo "ok\n";

    echo "getting K... ";
    flush();
    $k = $initial->getK();
    test($k->value->data == "l");
    echo "ok\n";

    echo "testing Value as parameter... ";
    flush();
    $v1 = new Test_L();
    $v1->data = "l";
    $v2 = null;
    $v3 = $initial->opValue($v1, $v2);
    test($v2->data == "l");
    test($v3->data == "l");

    $v1 = array(new Test_L());
    $v1[0]->data = "l";
    $v2 = null;
    $v3 = $initial->opValueSeq($v1, $v2);
    test($v2[0]->data == "l");
    test($v3[0]->data == "l");

    $v1 = array("l" => new Test_L());
    $v1["l"]->data = "l";
    $v2 = null;
    $v3 = $initial->opValueMap($v1, $v2);
    test($v2["l"]->data == "l");
    test($v3["l"]->data == "l");

    echo "ok\n";

    echo "getting D1... ";
    flush();
    $d1 = $initial->getD1(new Test_D1(new Test_A1("a1"), new Test_A1("a2"), new Test_A1("a3"), new Test_A1("a4")));
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
    catch(Exception $ex)
    {
        $ed = $NS ? "Test\\EDerived" : "Test_EDerived";
        if(!($ex instanceof $ed))
        {
            throw $ex;
        }
        test($ex->a1->name == "a1");
        test($ex->a2->name == "a2");
        test($ex->a3->name == "a3");
        test($ex->a4->name == "a4");
    }
    echo "ok\n";

    echo "setting G... ";
    flush();
    $cls = $NS ? "Test\\S" : "Test_S";
    try
    {
        $initial->setG(new Test_G(new $cls("hello"), "g"));
    }
    catch(Exception $ex)
    {
        $one = $NS ? "Ice\\OperationNotExistException" : "Ice_OperationNotExistException";
        if(!($ex instanceof $one))
        {
            throw $ex;
        }
    }
    echo "ok\n";

    echo "setting I... ";
    flush();
    $initial->setI($i);
    $initial->setI($j);
    $initial->setI($h);
    echo "ok\n";

    echo "testing sequences... ";
    flush();
    $outS = null;
    $initial->opBaseSeq(array(), $outS);

    $seq = array();
    for($i = 0; $i < 120; $i++)
    {
        $b = $NS ? eval("return new Test\\Base;") : eval("return new Test_Base;");
        $b->str = "b" . $i;
        $b->theS = $NS ? eval("return new Test\\S;") : eval("return new Test_S;");
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
    $top = new Test_Recursive();
    $p = $top;
    $depth = 0;
    try
    {
        while($depth <= 700)
        {
            $p->v = new Test_Recursive();
            $p = $p->v;
            if(($depth < 10 && ($depth % 10) == 0) ||
               ($depth < 1000 && ($depth % 100) == 0) ||
               ($depth < 10000 && ($depth % 1000) == 0) ||
               ($depth % 10000) == 0)
            {
                $initial->setRecursive($top);
            }
            $depth += 1;
        }
        test(!$initial->supportsClassGraphDepthMax());
    }
    catch(Exception $ex)
    {
        $ule = $NS ? "Ice\\UnknownLocalException" : "Ice_UnknownLocalException";
        $ue = $NS ? "Ice\\UnknownException" : "Ice_UnknownException";
        if($ex instanceof $ule)
        {
            // Expected marshal exception from the server (max class graph depth reached)
        }
        else if($ex instanceof $ue)
        {
            // Expected stack overflow from the server (Java only)
        }
        else
        {
            throw $ex;
        }
    }
    $initial->setRecursive(new Test_Recursive());
    echo "ok\n";

    echo "testing compact ID... ";
    flush();
    try
    {
        $r = $initial->getCompact();
        test($r != null);
    }
    catch(Exception $ex)
    {
        $one = $NS ? "Ice\\OperationNotExistException" : "Ice_OperationNotExistException";
        if(!($ex instanceof $one))
        {
            throw $ex;
        }
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
    $base = $communicator->stringToProxy($ref);
    test($base != null);
    $uoet = $base->ice_uncheckedCast("::Test::UnexpectedObjectExceptionTest");
    test($uoet != null);
    try
    {
        $uoet->op();
        test(false);
    }
    catch(Exception $ex)
    {
        $uoe = $NS ? "Ice\\UnexpectedObjectException" : "Ice_UnexpectedObjectException";
        $uoob = $NS ? "Ice\\UnmarshalOutOfBoundsException" : "Ice_UnmarshalOutOfBoundsException";
        if($ex instanceof $uoe)
        {
            test($ex->type == "::Test::AlsoEmpty");
            test($ex->expectedType == "::Test::Empty");
        }
        else if($ex instanceof $uoob)
        {
            //
            // We get UnmarshalOutOfBoundsException on Windows with VC6.
            //
        }
        else
        {
            throw $ex;
        }
    }
    echo "ok\n";

    echo "testing getting ObjectFactory... ";
    flush();
    test($communicator->findObjectFactory("TestOF") != null);
    echo "ok\n";

    echo "testing getting ObjectFactory as ValueFactory... ";
    flush();
    test($communicator->getValueFactoryManager()->find("TestOF") != null);
    echo "ok\n";

    echo "testing forward declarations... ";
    $f12 = null;
    $f11 = $initial->opF1(new Test_F1("F11"), $f12);
    test($f11->name == "F11");
    test($f12->name == "F12");

    $f22 = null;
    $ref = sprintf("F21:%s", $helper->getTestEndpoint());
    $f21 = $initial->opF2($communicator->stringToProxy($ref)->ice_uncheckedCast("::Test::F2"), $f22);
    test($f21->ice_getIdentity()->name == "F21");
    $f21->op();
    test($f22->ice_getIdentity()->name == "F22");

    if($initial->hasF3())
    {
        $f32 = null;
        $f31 = $initial->opF3(new Test_F3($f11, $f22), $f32);
        test($f31->f1->name == "F11");
        test($f31->f2->ice_getIdentity()->name = "F21");

        test($f32->f1->name == "F12");
        test($f32->f2->ice_getIdentity()->name = "F22");
    }
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
            $factory = new MyValueFactory();
            $communicator->getValueFactoryManager()->add($factory, "::Test::B");
            $communicator->getValueFactoryManager()->add($factory, "::Test::C");
            $communicator->getValueFactoryManager()->add($factory, "::Test::D");
            $communicator->getValueFactoryManager()->add($factory, "::Test::E");
            $communicator->getValueFactoryManager()->add($factory, "::Test::F");
            $communicator->getValueFactoryManager()->add($factory, "::Test::I");
            $communicator->getValueFactoryManager()->add($factory, "::Test::J");
            $communicator->getValueFactoryManager()->add($factory, "::Test::H");
            $communicator->addObjectFactory(new MyObjectFactory(), "TestOF");
            $initial = allTests($this);
            $initial->shutdown();
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
