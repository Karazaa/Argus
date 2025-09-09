// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

// Declaring a Singlecast, one-parameter void Delegate (done before classes)
DECLARE_DELEGATE(OnTickComplete)

class ArgusSystemsThread : public FRunnable 
{
public:
    ArgusSystemsThread();

    virtual bool Init() override { return true; };
    virtual uint32 Run() override;
    virtual void Exit() override {}
    virtual void Stop() override { m_isShutdown = true; }
    void TickThread() { m_tickCondition = true; }
    bool IsTicking() const { return m_tickCondition; }
    void StartThread();

private:
	FRunnableThread* m_thread;
    bool m_isShutdown = false;
    bool m_isStarted = false;
    std::atomic<bool> m_tickCondition = std::atomic<bool>(false);
    OnTickComplete m_onTickComplete;
};