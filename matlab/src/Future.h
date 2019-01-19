//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>

namespace IceMatlab
{

class Future
{
public:

    void token(std::function<void()>);

    enum State { Running, Sent, Finished };

    bool waitForState(State, double);
    bool waitForState(const std::string&, double);

    virtual void exception(std::exception_ptr);
    std::exception_ptr getException() const;

    virtual void sent();
    virtual std::string state() const;
    void cancel();

protected:

    virtual State stateImpl() const = 0;

    std::mutex _mutex;
    std::condition_variable _cond;

    std::function<void()> _token;
    std::exception_ptr _exception; // If a local exception occurs.
};

class SimpleFuture : public Future
{
public:

    SimpleFuture();

    void done();

protected:

    virtual State stateImpl() const;

private:

    State _state;
};

}
