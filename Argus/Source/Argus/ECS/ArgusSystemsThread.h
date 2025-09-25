// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

// Declaring a Singlecast, one-parameter void Delegate (done before classes)
DECLARE_DELEGATE(OnTickComplete)

class ArgusSystemsThread : public FRunnable 
{
public:
    ArgusSystemsThread();
    ~ArgusSystemsThread();

    virtual bool Init() override { return true; };
    virtual uint32 Run() override;
    virtual void Stop() override;
    void TickThread(float deltaTime) { m_deltaTime = deltaTime;  m_tickCondition = true; }
    void StartThread();

	bool IsTicking() const { return m_tickCondition; }
	bool IsShutdown() const { return m_isShutdown; }

private:
	FRunnableThread* m_thread;
    bool m_isShutdown = false;
    bool m_isStarted = false;
    float m_deltaTime = 0.01f;
    std::atomic<bool> m_tickCondition = std::atomic<bool>(false);
    OnTickComplete m_onTickComplete;
};