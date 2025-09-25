// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSystemsThread.h"
#include "GameTime.h"
#include "Systems/FogOfWarSystems.h"

ArgusSystemsThread::ArgusSystemsThread()
{
    m_thread = FRunnableThread::Create(this, TEXT("ArgusSystemsThread"));
}

ArgusSystemsThread::~ArgusSystemsThread()
{
    if (!m_thread)
    {
        return;
    }

    Stop();
    m_thread->Kill(true);
    delete m_thread;
    m_thread = nullptr;
}

void ArgusSystemsThread::StartThread()
{
	m_isStarted = true;
}

uint32 ArgusSystemsThread::Run()
{
    FPlatformProcess::ConditionalSleep([this]() -> bool
    {
        return m_isStarted;
    });

    
    while (!m_isShutdown)
    {
        FogOfWarSystems::RunThreadSystems(m_deltaTime);

        m_tickCondition = false;
        m_onTickComplete.ExecuteIfBound();
        FPlatformProcess::ConditionalSleep([this]() -> bool
        {
            return m_tickCondition || m_isShutdown;
        });
    }
    return 0;
}

void ArgusSystemsThread::Stop()
{
    m_isStarted = true;
    m_isShutdown = true;
}