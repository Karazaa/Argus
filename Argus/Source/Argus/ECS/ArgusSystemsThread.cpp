// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusSystemsThread.h"
#include "Systems/FogOfWarSystems.h"

ArgusSystemsThread::ArgusSystemsThread()
{
    m_thread = FRunnableThread::Create(this, TEXT("ArgusSystemsThread"));
}

void ArgusSystemsThread::StartThread()
{
    FogOfWarSystems::InitializeSystems();

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
        FogOfWarSystems::RunThreadSystems();

        m_tickCondition = false;
        FPlatformProcess::ConditionalSleep([this]() -> bool
        {
            return m_tickCondition;
        });
    }
    return 0;
}