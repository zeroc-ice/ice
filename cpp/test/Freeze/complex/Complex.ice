// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

module Complex
{

//
// The database key (the expression and the result). Naturally, this
// is a stupid key - but this is only a test :)
//
/*local*/ struct Key
{
    string expression;
    int result;
};

//
// A set of classes that represents a numeric parse tree.
//
/*local*/ class Node {
    int calc();
};

/*local*/ class NumberNode extends Node
{
    int calc();

    int _number;
};

/*local*/ class BinaryNode extends Node
{
    int calc();

    Node _left;
    Node _right;
};

/*local*/ class AddNode extends BinaryNode
{
    int calc();
};

/*local*/ class MultiplyNode extends BinaryNode
{
    int calc();
};

};
