<?
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
    $ref = "thrower:default -p 12345 -t 2000";
    $base = $ICE->stringToProxy($ref);
    test($base != null);
    echo "ok\n";

    echo "testing checked cast... ";
    flush();
    $thrower = $base->ice_checkedCast("::Thrower");
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
    catch(A $ex)
    {
        test($ex->aMem == 1);
    }

    try
    {
        $thrower->throwAorDasAorD(1);
        test(false);
    }
    catch(A $ex)
    {
        test($ex->aMem == 1);
    }

    try
    {
        $thrower->throwAorDasAorD(-1);
        test(false);
    }
    catch(D $ex)
    {
        test($ex->dMem == -1);
    }

    try
    {
        $thrower->throwBasB(1, 2);
        test(false);
    }
    catch(B $ex)
    {
        test($ex->aMem == 1);
        test($ex->bMem == 2);
    }

    try
    {
        $thrower->throwCasC(1, 2, 3);
        test(false);
    }
    catch(C $ex)
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
    catch(A $ex)
    {
        test($ex->aMem == 1);
    }

    try
    {
        $thrower->throwCasC(1, 2, 3);
        test(false);
    }
    catch(B $ex)
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
    catch(B $ex)
    {
        test($ex->aMem == 1);
        test($ex->bMem == 2);
    }

    try
    {
        $thrower->throwCasA(1, 2, 3);
        test(false);
    }
    catch(C $ex)
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
    catch(C $ex)
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
        $thrower2 = $thrower->ice_newIdentity($id)->ice_uncheckedCast("::Thrower");
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
        $thrower2 = $thrower->ice_uncheckedCast("::Thrower", "no such facet");
        try
        {
            $thrower2->ice_ping();
            test(false);
        }
        catch(Ice_FacetNotExistException $ex)
        {
            test(count($ex->facet) == 1);
            test($ex->facet[0] == "no such facet");
        }

        try
        {
            $thrower3 = $thrower2->ice_uncheckedCast("::Thrower", "no such facet either");
            $thrower3->ice_ping();
            test(false);
        }
        catch(Ice_FacetNotExistException $ex)
        {
            test(count($ex->facet) == 2);
            test($ex->facet[0] == "no such facet");
            test($ex->facet[1] == "no such facet either");
        }
    }

    echo "ok\n";

    echo "catching operation not exist exception... ";
    flush();

    try
    {
        $thrower2 = $thrower->ice_uncheckedCast("::WrongOperation");
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

try
{
    $thrower = allTests();
    $thrower->shutdown();
    exit();
}
catch(Ice_LocalException $ex)
{
    echo "Caught exception:\n";
    print_r($ex);
}
?>
