// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ComponentDependencies/TaskComponentStates.h"

class ITaskComponentObserver
{
public:
	virtual void OnChanged_m_baseState(BaseState oldValue, BaseState newValue) = 0;
};

class TaskComponentObservers
{
private:
	TArray<ITaskComponentObserver*> m_taskComponentObservers;

public:
	void AddObserver(ITaskComponentObserver* observer)
	{
		if (!observer)
		{
			return;
		}

		m_taskComponentObservers.Add(observer);
	}

	void RemoveObserver(ITaskComponentObserver* observer)
	{
		if (!observer)
		{
			return;
		}

		m_taskComponentObservers.RemoveAll
		(
			[observer] (ITaskComponentObserver* other)
			{
				return other == observer;
			}
		);
	}

private:
	void OnChanged_m_baseState(BaseState oldValue, BaseState newValue)
	{
		for (int32 i = 0; i < m_taskComponentObservers.Num(); ++i)
		{
			m_taskComponentObservers[i]->OnChanged_m_baseState(oldValue, newValue);
		}
	};

	friend struct TaskComponent;
};