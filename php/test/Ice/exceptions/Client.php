<?php
// Copyright (c) ZeroC, Inc.

require_once('Test.php');

function allTests($helper)
{
    $communicator = $helper->communicator();
    $ref = sprintf("thrower:%s", $helper->getTestEndpoint());
    $communicator = $helper->communicator();
    $thrower = Test\ThrowerPrxHelper::createProxy($communicator, $ref);

    echo "catching exact types... ";
    flush();

    try
    {
        $thrower->throwAasA(1);
        test(false);
    }
    catch(Test\A $ex)
    {
        test($ex->aMem == 1);
    }

    try
    {
        $thrower->throwAorDasAorD(1);
        test(false);
    }
    catch(Test\A $ex)
    {
        test($ex->aMem == 1);
    }

    try
    {
        $thrower->throwAorDasAorD(-1);
        test(false);
    }
    catch(Test\D $ex)
    {
        test($ex->dMem == -1);
    }

    try
    {
        $thrower->throwBasB(1, 2);
        test(false);
    }
    catch(Test\B $ex)
    {
        test($ex->aMem == 1);
        test($ex->bMem == 2);
    }

    try
    {
        $thrower->throwCasC(1, 2, 3);
        test(false);
    }
    catch(Test\C $ex)
    {
        test($ex->aMem == 1);
        test($ex->bMem == 2);
        test($ex->cMem == 3);
    }

    echo "ok\n";

    echo "catching base types... ";
    flush();

    try
    {
        $thrower->throwBasB(1, 2);
        test(false);
    }
    catch(Test\A $ex)
    {
        test($ex->aMem == 1);
    }

    try
    {
        $thrower->throwCasC(1, 2, 3);
        test(false);
    }
    catch(Test\B $ex)
    {
        test($ex->aMem == 1);
        test($ex->bMem == 2);
    }

    echo "ok\n";

    echo "catching derived types... ";
    flush();

    try
    {
        $thrower->throwBasA(1, 2);
        test(false);
    }
    catch(Test\B $ex)
    {
        test($ex->aMem == 1);
        test($ex->bMem == 2);
    }

    try
    {
        $thrower->throwCasA(1, 2, 3);
        test(false);
    }
    catch(Test\C $ex)
    {
        test($ex->aMem == 1);
        test($ex->bMem == 2);
        test($ex->cMem == 3);
    }

    try
    {
        $thrower->throwCasB(1, 2, 3);
        test(false);
    }
    catch(Test\C $ex)
    {
        test($ex->aMem == 1);
        test($ex->bMem == 2);
        test($ex->cMem == 3);
    }

    echo "ok\n";

    if($thrower->supportsUndeclaredExceptions())
    {
        echo "catching unknown user exception... ";
        flush();

        try
        {
            $thrower->throwUndeclaredA(1);
            test(false);
        }
        catch(Ice\UnknownUserException $ex)
        {
        }

        try
        {
            $thrower->throwUndeclaredB(1, 2);
            test(false);
        }
        catch(Ice\UnknownUserException $ex)
        {
        }

        try
        {
            $thrower->throwUndeclaredC(1, 2, 3);
            test(false);
        }
        catch(Ice\UnknownUserException $ex)
        {
        }

        echo "ok\n";
    }

    {
        echo "testing memory limit marshal exception...";
        flush();
        try
        {
            $thrower->throwMemoryLimitException(array(0x00));
            test(false);
        }
        catch(Ice\MarshalException $ex)
        {
        }

        try
        {
            $thrower->throwMemoryLimitException(array_pad(array(), 20 * 1024, 0x00));
            test(false);
        }
        catch(Exception $ex)
        {
            if(!($ex instanceof Ice\ConnectionLostException) && !($ex instanceof Ice\UnknownLocalException))
            {
                throw $ex;
            }
        }

        echo "ok\n";
    }

    echo "catching object not exist exception... ";
    flush();

    $id = Ice\stringToIdentity("does not exist");
    try
    {
        $thrower2 = Test\ThrowerPrxHelper::uncheckedCast($thrower->ice_identity($id));
        $thrower2->throwAasA(1);
        test(false);
    }
    catch(Ice\ObjectNotExistException $ex)
    {
        test(str_contains($ex->getMessage(), "Dispatch failed with ObjectNotExist"));
    }

    echo "ok\n";

    echo "catching facet not exist exception... ";
    flush();

    {
        $thrower2 = Test\ThrowerPrxHelper::uncheckedCast($thrower, "no such facet");
        try
        {
            $thrower2->ice_ping();
            test(false);
        }
        catch(Ice\FacetNotExistException $ex)
        {
            test($ex->facet == "no such facet");
        }
    }

    echo "ok\n";

    echo "catching operation not exist exception... ";
    flush();

    try
    {
        $thrower2 = Test\WrongOperationPrxHelper::uncheckedCast($thrower);
        $thrower2->noSuchOperation();
        test(false);
    }
    catch(Ice\OperationNotExistException $ex)
    {
        test($ex->operation = "noSuchOperation");
    }

    echo "ok\n";

    echo "catching unknown local exception... ";
    flush();

    try
    {
        $thrower->throwLocalException();
        test(false);
    }
    catch(Ice\UnknownLocalException $ex)
    {
    }

    try
    {
        $thrower->throwLocalExceptionIdempotent();
        test(false);
    }
    catch(Exception $ex)
    {
        if(!($ex instanceof Ice\UnknownLocalException) && !($ex instanceof Ice\OperationNotExistException))
        {
            throw $ex;
        }
    }

    echo "ok\n";

    echo "catching unknown non-Ice exception... ";
    flush();

    try
    {
        $thrower->throwNonIceException();
        test(false);
    }
    catch(Ice\UnknownException $ex)
    {
    }

    echo "ok\n";

    echo "catching dispatch exception... ";
    flush();

    try
    {
        $thrower->throwDispatchException(Ice\ReplyStatus::OperationNotExist);
        test(false);
    }
    catch(Ice\OperationNotExistException $ex)
    {
        test($ex->getMessage() == "Dispatch failed with OperationNotExist { id = 'thrower', facet = '', operation = 'throwDispatchException' }");
    }

    try
    {
        $thrower->throwDispatchException(Ice\ReplyStatus::Unauthorized);
        test(false);
    }
    catch(Ice\DispatchException $ex)
    {
        test($ex->replyStatus == Ice\ReplyStatus::Unauthorized);
        test($ex->getMessage() == "The dispatch failed with reply status Unauthorized." ||
            $ex->getMessage() == "The dispatch failed with reply status unauthorized."); # for Swift
    }

    try
    {
        $thrower->throwDispatchException(212);
        test(false);
    }
    catch(Ice\DispatchException $ex)
    {
        test($ex->replyStatus == 212);
        test($ex->getMessage() == "The dispatch failed with reply status 212.");
    }

    echo "ok\n";

    return $thrower;
}

class Client extends TestHelper
{
    function run($args)
    {
        try
        {
            $properties = $this->createTestProperties($args);
            $properties->setProperty("Ice.MessageSizeMax", "10");
            //
            // This property is set by the test suite, howerver we need to override it for this test.
            // Unlike C++, we can not pass $argv into Ice::createProperties, so we just set it after.
            //
            $properties->setProperty("Ice.Warn.Connections", "0");
            $communicator = $this->initialize($properties);
            $proxy= allTests($this);
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
