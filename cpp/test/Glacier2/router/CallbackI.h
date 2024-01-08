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

    void callback(const Ice::Current&) override;
    void callbackEx(const Ice::Current&) override;
    void concurrentCallbackAsync(int, std::function<void(int)>, std::function<void(std::exception_ptr)>,
                                 const ::Ice::Current&) override;

    void waitCallback(const ::Ice::Current&) override;
    void callbackWithPayload(Ice::ByteSeq, const ::Ice::Current&) override;

    void callbackOK(int = 1);
    void waitCallbackOK();
    void callbackWithPayloadOK();
    void notifyWaitCallback();
    void answerConcurrentCallbacks(unsigned int);

private:

    int _callback = 0;
    bool _waitCallback = false;
    bool _callbackWithPayload = false;
    bool _finishWaitCallback = false;
    std::vector<std::tuple<std::function<void(int)>, std::function<void(std::exception_ptr)>, int>> _callbacks;

    std::mutex _mutex;
    std::condition_variable _condVar;
};

class CallbackI final : public ::Test::Callback
{
public:

    void initiateCallbackAsync(std::shared_ptr<Test::CallbackReceiverPrx>,
                               std::function<void()>, std::function<void(std::exception_ptr)>,
                               const Ice::Current&) override;

    void initiateCallbackExAsync(std::shared_ptr<Test::CallbackReceiverPrx>,
                                 std::function<void()>, std::function<void(std::exception_ptr)>,
                                 const Ice::Current&) override;

    void initiateConcurrentCallbackAsync(int, std::shared_ptr<Test::CallbackReceiverPrx>,
                                         std::function<void(int)>, std::function<void(std::exception_ptr)>,
                                         const Ice::Current&) override;

    void initiateWaitCallbackAsync(std::shared_ptr<Test::CallbackReceiverPrx>,
                                   std::function<void()>, std::function<void(std::exception_ptr)>,
                                   const Ice::Current&) override;

    void initiateCallbackWithPayloadAsync(std::shared_ptr<Test::CallbackReceiverPrx>,
                                          std::function<void()>, std::function<void(std::exception_ptr)>,
                                          const Ice::Current&) override;

    void shutdown(const Ice::Current&) override;
};

#endif
