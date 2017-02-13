<?
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
require_once ($NS ? 'Ice_ns.php' : 'Ice.php');
require_once 'Test.php';

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
        interface Test_I extends Test\I {}
        interface Test_J extends Test\J {}
        class Test_H extends Test\H {}
        class Ice_ObjectImpl extends Ice\ObjectImpl {}
        interface Ice_ObjectFactory extends Ice\ObjectFactory {}
EOT;
    eval($code);
}

class BI extends Test_B
{
    function ice_postUnmarshal()
    {
        $this->_postUnmarshalInvoked = true;
    }

    function postUnmarshalInvoked()
    {
        return $this->_postUnmarshalInvoked;
    }

    private $_postUnmarshalInvoked = false;
}

class CI extends Test_C
{
    function ice_postUnmarshal()
    {
        $this->_postUnmarshalInvoked = true;
    }

    function postUnmarshalInvoked()
    {
        return $this->_postUnmarshalInvoked;
    }

    private $_postUnmarshalInvoked = false;
}

class DI extends Test_D
{
    function ice_postUnmarshal()
    {
        $this->_postUnmarshalInvoked = true;
    }

    function postUnmarshalInvoked()
    {
        return $this->_postUnmarshalInvoked;
    }

    private $_postUnmarshalInvoked = false;
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

class II extends Ice_ObjectImpl implements Test_I
{
}

class JI extends Ice_ObjectImpl implements Test_J
{
}

class HI extends Test_H
{
}

class MyObjectFactory implements Ice_ObjectFactory
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

    function destroy()
    {
    }
}

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

    echo "testing stringToProxy... ";
    flush();
    $ref = "initial:default -p 12010";
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
    test($b1->postUnmarshalInvoked());
    test($b1->theA->preMarshalInvoked);
    test($b1->theA->postUnmarshalInvoked());
    test($b1->theA->theC->preMarshalInvoked);
    test($b1->theA->theC->postUnmarshalInvoked());
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
    test($d->postUnmarshalInvoked());
    test($d->theA->preMarshalInvoked);
    test($d->theA->postUnmarshalInvoked());
    test($d->theB->preMarshalInvoked);
    test($d->theB->postUnmarshalInvoked());
    test($d->theB->theC->preMarshalInvoked);
    test($d->theB->theC->postUnmarshalInvoked());
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
    test($j != null and $j instanceof Test_J);
    $h = $initial->getH();
    test($h != null and $h instanceof Test_H);
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

    $base = $NS ? eval("return new Test\\Base;") : eval("return new Test_Base;");
    $retS = $initial->opBaseSeq(array($base), $outS);
    test(count($retS) == 1 && count($outS) == 1);
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

    echo "testing UnexpectedObjectException... ";
    flush();
    $ref = "uoet:default -p 12010";
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

    return $initial;
}

$communicator = Ice_initialize($argv);
$factory = new MyObjectFactory();
$communicator->addObjectFactory($factory, "::Test::B");
$communicator->addObjectFactory($factory, "::Test::C");
$communicator->addObjectFactory($factory, "::Test::D");
$communicator->addObjectFactory($factory, "::Test::E");
$communicator->addObjectFactory($factory, "::Test::F");
$communicator->addObjectFactory($factory, "::Test::I");
$communicator->addObjectFactory($factory, "::Test::J");
$communicator->addObjectFactory($factory, "::Test::H");
$initial = allTests($communicator);
$initial->shutdown();
$communicator->destroy();
exit();
?>
