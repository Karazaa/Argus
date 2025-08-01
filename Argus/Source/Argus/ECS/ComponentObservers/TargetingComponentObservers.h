// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ArgusContainerAllocator.h"
#include "CoreMinimal.h"
#include "ComponentDependencies/TaskComponentStates.h"

class ITargetingComponentObserver
{
public:
};

class TargetingComponentObservers
{
private:
	TArray<ITargetingComponentObserver*, ArgusContainerAllocator<2> > m_TargetingComponentObservers;

public:
	void AddObserver(ITargetingComponentObserver* observer)
	{
		if (!observer)
		{
			return;
		}

		m_TargetingComponentObservers.Add(observer);
	}

	void RemoveObserver(ITargetingComponentObserver* observer)
	{
		if (!observer)
		{
			return;
		}

		m_TargetingComponentObservers.RemoveAll
		(
			[observer] (ITargetingComponentObserver* other)
			{
				return other == observer;
			}
		);
	}

private:

	friend struct TargetingComponent;
};
