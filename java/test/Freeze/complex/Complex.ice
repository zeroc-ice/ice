// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
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
struct Key
{
    string expression;
    int result;
};

//
// A set of classes that represents a numeric parse tree.
//
class Node {
    int calc();
};

class NumberNode extends Node
{
    int number;
};

class BinaryNode extends Node
{
    Node left;
    Node right;
};

class AddNode extends BinaryNode
{
};

class MultiplyNode extends BinaryNode
{
};

};
