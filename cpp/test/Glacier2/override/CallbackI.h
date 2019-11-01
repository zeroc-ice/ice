//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef CALLBACK_I_H
#define CALLBACK_I_H

#include <Callback.h>
#include <vector>

class CallbackReceiverI final : public Test::CallbackReceiver
{
public:

    void callback(int token, const Ice::Current&) override;
    void callbackWithPayload(Ice::ByteSeq, const ::Ice::Current&) override;

    int callbackOK(int, int);
    int callbackWithPayloadOK(int);

    void hold();
    void activate();

private:

    void checkForHold(std::unique_lock<std::mutex>&);

    bool _holding = false;

    int _lastToken = -1;
    int _callback = 0;
    int _callbackWithPayload = 0;

    std::mutex _mutex;
    std::condition_variable _condVar;
};

class CallbackI final : public Test::Callback
{
public:

    void initiateCallbackAsync(std::shared_ptr<Test::CallbackReceiverPrx>, int,
                               std::function<void()>, std::function<void(std::exception_ptr)>,
                               const Ice::Current&) override;
    void initiateCallbackWithPayloadAsync(std::shared_ptr<Test::CallbackReceiverPrx>,
                                          std::function<void()>, std::function<void(std::exception_ptr)>,
                                          const ::Ice::Current&) override;
    void shutdown(const Ice::Current&) override;
};

#endif
