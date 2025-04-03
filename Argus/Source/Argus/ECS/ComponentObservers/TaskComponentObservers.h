// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "CoreMinimal.h"
#include "ComponentDependencies/TaskComponentStates.h"

class ITaskComponentObserver
{
public:
	virtual void OnChanged_m_baseState(EBaseState oldValue, EBaseState newValue) = 0;
};

class TaskComponentObservers
{
private:
	TArray<ITaskComponentObserver*> m_TaskComponentObservers;

public:
	void AddObserver(ITaskComponentObserver* observer)
	{
		if (!observer)
		{
			return;
		}

		m_TaskComponentObservers.Add(observer);
	}

	void RemoveObserver(ITaskComponentObserver* observer)
	{
		if (!observer)
		{
			return;
		}

		m_TaskComponentObservers.RemoveAll
		(
			[observer] (ITaskComponentObserver* other)
			{
				return other == observer;
			}
		);
	}

private:
	void OnChanged_m_baseState(EBaseState oldValue, EBaseState newValue)
	{
		for (int32 i = 0; i < m_TaskComponentObservers.Num(); ++i)
		{
			m_TaskComponentObservers[i]->OnChanged_m_baseState(oldValue, newValue);
		}
	};

	friend struct TaskComponent;
};
