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

function allTests()
{
    global $ICE;

    echo "testing stringToProxy... ";
    flush();
    $ref = "thrower:default -p 12010 -t 2000";
    $base = $ICE->stringToProxy($ref);
    test($base != null);
    echo "ok\n";

    echo "testing checked cast... ";
    flush();
    $thrower = $base->ice_checkedCast("::Test::Thrower");
    test($thrower != null);
    test($thrower == $base);
    echo "ok\n";

    echo "catching exact types... ";
    flush();

    try
    {
        $thrower->throwAasA(1);
        test(false);
    }
    catch(Test_A $ex)
    {
        test($ex->aMem == 1);
    }

    try
    {
        $thrower->throwAorDasAorD(1);
        test(false);
    }
    catch(Test_A $ex)
    {
        test($ex->aMem == 1);
    }

    try
    {
        $thrower->throwAorDasAorD(-1);
        test(false);
    }
    catch(Test_D $ex)
    {
        test($ex->dMem == -1);
    }

    try
    {
        $thrower->throwBasB(1, 2);
        test(false);
    }
    catch(Test_B $ex)
    {
        test($ex->aMem == 1);
        test($ex->bMem == 2);
    }

    try
    {
        $thrower->throwCasC(1, 2, 3);
        test(false);
    }
    catch(Test_C $ex)
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
    catch(Test_A $ex)
    {
        test($ex->aMem == 1);
    }

    try
    {
        $thrower->throwCasC(1, 2, 3);
        test(false);
    }
    catch(Test_B $ex)
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
    catch(Test_B $ex)
    {
        test($ex->aMem == 1);
        test($ex->bMem == 2);
    }

    try
    {
        $thrower->throwCasA(1, 2, 3);
        test(false);
    }
    catch(Test_C $ex)
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
    catch(Test_C $ex)
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
        catch(Ice_UnknownUserException $ex)
        {
        }

        try
        {
            $thrower->throwUndeclaredB(1, 2);
            test(false);
        }
        catch(Ice_UnknownUserException $ex)
        {
        }

        try
        {
            $thrower->throwUndeclaredC(1, 2, 3);
            test(false);
        }
        catch(Ice_UnknownUserException $ex)
        {
        }

        echo "ok\n";
    }

    echo "catching object not exist exception... ";
    flush();

    $id = Ice_stringToIdentity("does not exist");
    try
    {
        $thrower2 = $thrower->ice_identity($id)->ice_uncheckedCast("::Test::Thrower");
        $thrower2->throwAasA(1);
        test(false);
    }
    catch(Ice_ObjectNotExistException $ex)
    {
        test($ex->id == $id);
    }

    echo "ok\n";

    echo "catching facet not exist exception... ";
    flush();

    {
        $thrower2 = $thrower->ice_uncheckedCast("::Test::Thrower", "no such facet");
        try
        {
            $thrower2->ice_ping();
            test(false);
        }
        catch(Ice_FacetNotExistException $ex)
        {
            test($ex->facet == "no such facet");
        }
    }

    echo "ok\n";

    echo "catching operation not exist exception... ";
    flush();

    try
    {
        $thrower2 = $thrower->ice_uncheckedCast("::Test::WrongOperation");
        $thrower2->noSuchOperation();
        test(false);
    }
    catch(Ice_OperationNotExistException $ex)
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
    catch(Ice_UnknownLocalException $ex)
    {
    }

    echo "ok\n";

    echo "catching unknown non-Ice exception... ";
    flush();

    try
    {
        $thrower->throwNonIceException();
        test(false);
    }
    catch(Ice_UnknownException $ex)
    {
    }

    echo "ok\n";

    return $thrower;
}

$thrower = allTests();
$thrower->shutdown();
exit();
?>
