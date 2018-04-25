<?php
// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

function allTests()
{
    global $NS;

    $red = $NS ? constant("Test\\Color::red") : constant("Test_Color::red");
    $green = $NS ? constant("Test\\Color::green") : constant("Test_Color::green");
    $blue = $NS ? constant("Test\\Color::blue") : constant("Test_Color::blue");

    $nRed = $NS ? constant("Test\\Nested\\Color::red") : constant("Test_Nested_Color::red");
    $nGreen = $NS ? constant("Test\\Nested\\Color::green") : constant("Test_Nested_Color::green");
    $nBlue = $NS ? constant("Test\\Nested\\Color::blue") : constant("Test_Nested_Color::blue");

    echo "testing default values... ";
    flush();

    {
        $v = $NS ? eval("return new Test\\Struct1;") : eval("return new Test_Struct1;");
        test(!$v->boolFalse);
        test($v->boolTrue);
        test($v->b == 254);
        test($v->s == 16000);
        test($v->i == 3);
        test($v->l == 4);
        test($v->f == 5.1);
        test($v->d == 6.2);
        test($v->str == "foo \\ \"bar\n \r\n\t\013\f\007\010? \007 \007");
        test($v->c1 == $red);
        test($v->c2 == $green);
        test($v->c3 == $blue);
        test($v->nc1 == $nRed);
        test($v->nc2 == $nGreen);
        test($v->nc3 == $nBlue);
        test(strlen($v->noDefault) == 0);
        test($v->zeroI == 0);
        test($v->zeroL == 0);
        test($v->zeroF == 0);
        test($v->zeroDotF == 0);
        test($v->zeroD == 0);
        test($v->zeroDotD == 0);
    }

    {
        $v = $NS ? eval("return new Test\\Struct2;") : eval("return new Test_Struct2;");
        test($v->boolTrue == constant($NS ? "Test\\ConstBool" : "Test_ConstBool"));
        test($v->b == constant($NS ? "Test\\ConstByte" : "Test_ConstByte"));
        test($v->s == constant($NS ? "Test\\ConstShort" : "Test_ConstShort"));
        test($v->i == constant($NS ? "Test\\ConstInt" : "Test_ConstInt"));
        test($v->l == constant($NS ? "Test\\ConstLong" : "Test_ConstLong"));
        test($v->f == constant($NS ? "Test\\ConstFloat" : "Test_ConstFloat"));
        test($v->d == constant($NS ? "Test\\ConstDouble" : "Test_ConstDouble"));
        test($v->str == constant($NS ? "Test\\ConstString" : "Test_ConstString"));
        test($v->c1 == constant($NS ? "Test\\ConstColor1" : "Test_ConstColor1"));
        test($v->c2 == constant($NS ? "Test\\ConstColor2" : "Test_ConstColor2"));
        test($v->c3 == constant($NS ? "Test\\ConstColor3" : "Test_ConstColor3"));
        test($v->nc1 == constant($NS ? "Test\\ConstNestedColor1" : "Test_ConstNestedColor1"));
        test($v->nc2 == constant($NS ? "Test\\ConstNestedColor2" : "Test_ConstNestedColor2"));
        test($v->nc3 == constant($NS ? "Test\\ConstNestedColor3" : "Test_ConstNestedColor3"));
        test($v->zeroI == constant($NS ? "Test\\ConstZeroI" : "Test_ConstZeroI"));
        test($v->zeroL == constant($NS ? "Test\\ConstZeroL" : "Test_ConstZeroL"));
        test($v->zeroF == constant($NS ? "Test\\ConstZeroF" : "Test_ConstZeroF"));
        test($v->zeroDotF == constant($NS ? "Test\\ConstZeroDotF" : "Test_ConstZeroDotF"));
        test($v->zeroD == constant($NS ? "Test\\ConstZeroD" : "Test_ConstZeroD"));
        test($v->zeroDotD == constant($NS ? "Test\\ConstZeroDotD" : "Test_ConstZeroDotD"));
    }

    {
        $v = $NS ? eval("return new Test\\Base;") : eval("return new Test_Base;");
        test(!$v->boolFalse);
        test($v->boolTrue);
        test($v->b == 1);
        test($v->s == 2);
        test($v->i == 3);
        test($v->l == 4);
        test($v->f == 5.1);
        test($v->d == 6.2);
        test($v->str == "foo \\ \"bar\n \r\n\t\013\f\007\010? \007 \007");
        test(strlen($v->noDefault) == 0);
        test($v->zeroI == 0);
        test($v->zeroL == 0);
        test($v->zeroF == 0);
        test($v->zeroDotF == 0);
        test($v->zeroD == 0);
        test($v->zeroDotD == 0);
    }

    {
        $v = $NS ? eval("return new Test\\Derived;") : eval("return new Test_Derived;");
        test(!$v->boolFalse);
        test($v->boolTrue);
        test($v->b == 1);
        test($v->s == 2);
        test($v->i == 3);
        test($v->l == 4);
        test($v->f == 5.1);
        test($v->d == 6.2);
        test($v->str == "foo \\ \"bar\n \r\n\t\013\f\007\010? \007 \007");
        test(strlen($v->noDefault) == 0);
        test($v->c1 == $red);
        test($v->c2 == $green);
        test($v->c3 == $blue);
        test($v->nc1 == $nRed);
        test($v->nc2 == $nGreen);
        test($v->nc3 == $nBlue);
        test($v->zeroI == 0);
        test($v->zeroL == 0);
        test($v->zeroF == 0);
        test($v->zeroDotF == 0);
        test($v->zeroD == 0);
        test($v->zeroDotD == 0);
    }

    {
        $v = $NS ? eval("return new Test\\BaseEx;") : eval("return new Test_BaseEx;");
        test(!$v->boolFalse);
        test($v->boolTrue);
        test($v->b == 1);
        test($v->s == 2);
        test($v->i == 3);
        test($v->l == 4);
        test($v->f == 5.1);
        test($v->d == 6.2);
        test($v->str == "foo \\ \"bar\n \r\n\t\013\f\007\010? \007 \007");
        test(strlen($v->noDefault) == 0);
        test($v->zeroI == 0);
        test($v->zeroL == 0);
        test($v->zeroF == 0);
        test($v->zeroDotF == 0);
        test($v->zeroD == 0);
        test($v->zeroDotD == 0);
    }

    {
        $v = $NS ? eval("return new Test\\DerivedEx;") : eval("return new Test_DerivedEx;");
        test(!$v->boolFalse);
        test($v->boolTrue);
        test($v->b == 1);
        test($v->s == 2);
        test($v->i == 3);
        test($v->l == 4);
        test($v->f == 5.1);
        test($v->d == 6.2);
        test($v->str == "foo \\ \"bar\n \r\n\t\013\f\007\010? \007 \007");
        test(strlen($v->noDefault) == 0);
        test($v->c1 == $red);
        test($v->c2 == $green);
        test($v->c3 == $blue);
        test($v->nc1 == $nRed);
        test($v->nc2 == $nGreen);
        test($v->nc3 == $nBlue);
        test($v->zeroI == 0);
        test($v->zeroL == 0);
        test($v->zeroF == 0);
        test($v->zeroDotF == 0);
        test($v->zeroD == 0);
        test($v->zeroDotD == 0);
    }
    echo "ok\n";

    echo "testing default constructor... ";
    flush();
    {
        $v = $NS ? eval("return new Test\\StructNoDefaults;") : eval("return new Test_StructNoDefaults;");
        $innerStructClass = $NS ? "Test\\InnerStruct" : "Test_InnerStruct";
        test($v->bo == false);
        test($v->b == 0);
        test($v->s == 0);
        test($v->i == 0);
        test($v->l == 0);
        test($v->f == 0.0);
        test($v->d == 0.0);
        test($v->str == "");
        test($v->c1 == $red);
        test($v->bs == null);
        test($v->is == null);
        test($v->dict == null);
        test($v->st instanceof $innerStructClass);

        $e = $NS ? eval("return new Test\\ExceptionNoDefaults;") : eval("return new Test_ExceptionNoDefaults;");
        test($e->str == '');
        test($e->c1 == $red);
        test($e->bs == null);
        test($e->st instanceof $innerStructClass);
        test($e->dict == null);

        $cl = $NS ? eval("return new Test\\ClassNoDefaults;") : eval("return new Test_ClassNoDefaults;");
        test($cl->str == '');
        test($cl->c1 == $red);
        test($cl->bs == null);
        test($cl->st instanceof $innerStructClass);
        test($cl->dict == null);
    }
    echo "ok\n";
}

allTests();

exit();
?>
