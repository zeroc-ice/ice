<?php
// Copyright (c) ZeroC, Inc.

require_once('Test.php');

function allTests()
{
    echo "testing default values... ";
    flush();

    {
        $v = new Test\Struct1();
        test(!$v->boolFalse);
        test($v->boolTrue);
        test($v->b == 254);
        test($v->s == 16000);
        test($v->i == 3);
        test($v->l == 4);
        test($v->f == 5.1);
        test($v->d == 6.2);
        test($v->str == "foo \ \"bar\n \r\n\t\013\f\007\010? \007 \007");
        test($v->c1 == Test\Color::red);
        test($v->c2 == Test\Color::green);
        test($v->c3 == Test\Color::blue);
        test($v->nc1 == Test\Nested\Color::red);
        test($v->nc2 == Test\Nested\Color::green);
        test($v->nc3 == Test\Nested\Color::blue);
        test(strlen($v->noDefault) == 0);
        test($v->zeroI == 0);
        test($v->zeroL == 0);
        test($v->zeroF == 0);
        test($v->zeroDotF == 0);
        test($v->zeroD == 0);
        test($v->zeroDotD == 0);
    }

    {
        $v = new Test\Struct2();
        test($v->boolTrue == Test\ConstBool);
        test($v->b == Test\ConstByte);
        test($v->s == Test\ConstShort);
        test($v->i == Test\ConstInt);
        test($v->l == Test\ConstLong);
        test($v->f == Test\ConstFloat);
        test($v->d == Test\ConstDouble);
        test($v->str == Test\ConstString);
        test($v->c1 == Test\ConstColor1);
        test($v->c2 == Test\ConstColor2);
        test($v->c3 == Test\ConstColor3);
        test($v->nc1 == Test\ConstNestedColor1);
        test($v->nc2 == Test\ConstNestedColor2);
        test($v->nc3 == Test\ConstNestedColor3);
        test($v->zeroI == Test\ConstZeroI);
        test($v->zeroL == Test\ConstZeroL);
        test($v->zeroF == Test\ConstZeroF);
        test($v->zeroDotF == Test\ConstZeroDotF);
        test($v->zeroD == Test\ConstZeroD);
        test($v->zeroDotD == Test\ConstZeroDotD);
    }

    {
        $v = new Test\Base();
        test(!$v->boolFalse);
        test($v->boolTrue);
        test($v->b == 1);
        test($v->s == 2);
        test($v->i == 3);
        test($v->l == 4);
        test($v->f == 5.1);
        test($v->d == 6.2);
        test($v->str == "foo \ \"bar\n \r\n\t\013\f\007\010? \007 \007");
        test(strlen($v->noDefault) == 0);
        test($v->zeroI == 0);
        test($v->zeroL == 0);
        test($v->zeroF == 0);
        test($v->zeroDotF == 0);
        test($v->zeroD == 0);
        test($v->zeroDotD == 0);
    }

    {
        $v = new Test\Derived();
        test(!$v->boolFalse);
        test($v->boolTrue);
        test($v->b == 1);
        test($v->s == 2);
        test($v->i == 3);
        test($v->l == 4);
        test($v->f == 5.1);
        test($v->d == 6.2);
        test($v->str == "foo \ \"bar\n \r\n\t\013\f\007\010? \007 \007");
        test(strlen($v->noDefault) == 0);
        test($v->c1 == Test\Color::red);
        test($v->c2 == Test\Color::green);
        test($v->c3 == Test\Color::blue);
        test($v->nc1 == Test\Nested\Color::red);
        test($v->nc2 == Test\Nested\Color::green);
        test($v->nc3 == Test\Nested\Color::blue);
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
        $v = new Test\StructNoDefaults();
        test($v->bo == false);
        test($v->b == 0);
        test($v->s == 0);
        test($v->i == 0);
        test($v->l == 0);
        test($v->f == 0.0);
        test($v->d == 0.0);
        test($v->str == "");
        test($v->c1 == Test\Color::red);
        test($v->bs == null);
        test($v->is == null);
        test($v->dict == null);
        test($v->st instanceof Test\InnerStruct);

        $cl = new Test\ClassNoDefaults();
        test($cl->str == '');
        test($cl->c1 == Test\Color::red);
        test($cl->bs == null);
        test($cl->st instanceof Test\InnerStruct);
        test($cl->dict == null);
    }
    echo "ok\n";
}

class Client extends TestHelper
{
    function run($args)
    {
        allTests();
    }
}
?>
