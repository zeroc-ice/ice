// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
