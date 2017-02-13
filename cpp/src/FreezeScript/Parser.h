// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_SCRIPT_PARSER_H
#define FREEZE_SCRIPT_PARSER_H

#include <FreezeScript/Data.h>

namespace FreezeScript
{

class SymbolTable;
typedef IceUtil::Handle<SymbolTable> SymbolTablePtr;

class Node;
typedef IceUtil::Handle<Node> NodePtr;

typedef std::vector<NodePtr> NodeList;

class EntityNode;
typedef IceUtil::Handle<EntityNode> EntityNodePtr;

//
// parseExpression parses the given expression and returns the root node of the parse tree.
//
NodePtr parseExpression(const std::string&, const DataFactoryPtr&, const ErrorReporterPtr&);

//
// EvaluateException is raised by Node::evaluate().
//
class EvaluateException : public IceUtil::Exception
{
public:
    
    EvaluateException(const char*, int, const std::string&);
    virtual ~EvaluateException() throw();
    virtual std::string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual EvaluateException* ice_clone() const;
    virtual void ice_throw() const;

    std::string reason() const;

private:

    std::string _reason;
    static const char* _name;
};

//
// SymbolTable is an interface for the lookup operations required by node evaluation.
//
class SymbolTable : public IceUtil::SimpleShared
{
public:

    virtual ~SymbolTable();

    virtual void add(const std::string&, const DataPtr&) = 0;

    virtual DataPtr getValue(const EntityNodePtr&) const = 0;

    virtual DataPtr getConstantValue(const std::string&) const = 0;

    virtual SymbolTablePtr createChild() = 0;

    virtual Slice::TypePtr lookupType(const std::string&) = 0;

    virtual DataPtr invokeFunction(const std::string&, const DataPtr&, const DataList&) = 0;
};

class Node : public IceUtil::SimpleShared
{
public:

    virtual ~Node();

    virtual DataPtr evaluate(const SymbolTablePtr&) = 0;

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

    virtual DataPtr evaluate(const SymbolTablePtr&);

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

    virtual DataPtr evaluate(const SymbolTablePtr&);

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

    virtual DataPtr evaluate(const SymbolTablePtr&);

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

    virtual DataPtr evaluate(const SymbolTablePtr&);

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

class FunctionNode : public Node
{
public:

    FunctionNode(const std::string&, const NodeList&);

    virtual DataPtr evaluate(const SymbolTablePtr&);

    virtual void print(std::ostream&) const;

    void setTarget(const EntityNodePtr&);

private:

    std::string _name;
    NodeList _args;
    EntityNodePtr _target;
};
typedef IceUtil::Handle<FunctionNode> FunctionNodePtr;

class ConstantNode : public Node
{
public:

    ConstantNode(const std::string&);

    virtual DataPtr evaluate(const SymbolTablePtr&);

    virtual void print(std::ostream&) const;

private:

    std::string _value;
};

} // End of namespace FreezeScript

std::ostream& operator<<(std::ostream&, const FreezeScript::EntityNodePtr&);

#endif
