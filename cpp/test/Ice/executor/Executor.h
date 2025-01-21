// Copyright (c) ZeroC, Inc.

#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "Ice/Ice.h"

#include <deque>

class ExecutorCall
{
public:
    ExecutorCall(std::function<void()> call) : _call(std::move(call)) {}

    void run() { _call(); }

private:
    std::function<void()> _call;
};

class Executor
{
public:
    static std::shared_ptr<Executor> create();
    void execute(const std::shared_ptr<ExecutorCall>&, const Ice::ConnectionPtr&);

    void run();
    void terminate();

    static bool isExecutorThread();

private:
    Executor();
    static std::shared_ptr<Executor> _instance;
    std::deque<std::shared_ptr<ExecutorCall>> _calls;
    bool _terminated;
    std::mutex _mutex;
    std::condition_variable _conditionVariable;
};

#endif
