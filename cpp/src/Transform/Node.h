// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef TRANSFORM_NODE_H
#define TRANSFORM_NODE_H

#include <Transform/Data.h>

namespace Transform
{

class Node;
typedef IceUtil::Handle<Node> NodePtr;

class EntityNode;
typedef IceUtil::Handle<EntityNode> EntityNodePtr;

class EvaluateException : public IceUtil::Exception
{
public:
    
    EvaluateException(const char*, int, const std::string&);
    virtual const std::string& ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;

    std::string reason() const;

private:

    std::string _reason;
    static std::string _name;
};

class SymbolTable
{
public:

    virtual ~SymbolTable();

    virtual DataPtr getValue(const EntityNodePtr&) const = 0;
    virtual DataPtr getConstantValue(const std::string&) const = 0;
};

class Node : public IceUtil::SimpleShared
{
public:

    virtual ~Node();

    virtual DataPtr evaluate(SymbolTable&) = 0;

    virtual void print(std::ostream&) const = 0;
};

enum BinaryOperator
{
    BinOpOr, BinOpAnd,
    BinOpMul, BinOpDiv, BinOpMod,
    BinOpAdd, BinOpSub,
    BinOpLess, BinOpGreater, BinOpLessEq, BinOpGrEq, BinOpEq, BinOpNotEq
};

class BinaryNode : public Node
{
public:

    BinaryNode(BinaryOperator, const DataFactoryPtr&, const NodePtr&, const NodePtr&);

    virtual DataPtr evaluate(SymbolTable&);

    virtual void print(std::ostream&) const;

private:

    static std::string opToString(BinaryOperator);

    BinaryOperator _op;
    DataFactoryPtr _factory;
    NodePtr _left;
    NodePtr _right;
};

enum UnaryOperator { UnaryOpNeg, UnaryOpNot };

class UnaryNode : public Node
{
public:

    UnaryNode(UnaryOperator, const DataFactoryPtr&, const NodePtr&);

    virtual DataPtr evaluate(SymbolTable&);

    virtual void print(std::ostream&) const;

private:

    static std::string opToString(UnaryOperator);

    UnaryOperator _op;
    DataFactoryPtr _factory;
    NodePtr _right;
};

class DataNode : public Node
{
public:

    DataNode(const DataPtr&);

    virtual DataPtr evaluate(SymbolTable&);

    virtual void print(std::ostream&) const;

private:

    DataPtr _data;
};

class EntityNodeVisitor
{
public:

    virtual ~EntityNodeVisitor();

    virtual void visitIdentifier(const std::string&) = 0;
    virtual void visitElement(const NodePtr&) = 0;
};

class EntityNode : public Node
{
public:

    virtual DataPtr evaluate(SymbolTable&);

    virtual void print(std::ostream&) const;

    virtual void visit(EntityNodeVisitor&) const = 0;

    void append(const EntityNodePtr&);

protected:

    EntityNodePtr _next;
};

class IdentNode : public EntityNode
{
public:

    IdentNode(const std::string&);

    virtual void visit(EntityNodeVisitor&) const;

    std::string getValue() const;

private:

    std::string _value;
};
typedef IceUtil::Handle<IdentNode> IdentNodePtr;

class ElementNode : public EntityNode
{
public:

    ElementNode(const NodePtr&);

    virtual void visit(EntityNodeVisitor&) const;

    NodePtr getValue() const;

private:

    NodePtr _value;
};
typedef IceUtil::Handle<ElementNode> ElementNodePtr;

class ConstantNode : public Node
{
public:

    ConstantNode(const std::string&);

    virtual DataPtr evaluate(SymbolTable&);

    virtual void print(std::ostream&) const;

private:

    std::string _value;
};

} // End of namespace Transform

std::ostream& operator<<(std::ostream&, const Transform::EntityNodePtr&);

#endif
