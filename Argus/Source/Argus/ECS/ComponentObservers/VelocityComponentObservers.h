// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "ArgusContainerAllocator.h"
#include "CoreMinimal.h"
#include "ComponentDependencies/TaskComponentStates.h"

class IVelocityComponentObserver
{
public:
};

class VelocityComponentObservers
{
private:
	TArray<IVelocityComponentObserver*, ArgusContainerAllocator<2> > m_VelocityComponentObservers;

public:
	void AddObserver(IVelocityComponentObserver* observer)
	{
		if (!observer)
		{
			return;
		}

		m_VelocityComponentObservers.Add(observer);
	}

	void RemoveObserver(IVelocityComponentObserver* observer)
	{
		if (!observer)
		{
			return;
		}

		m_VelocityComponentObservers.RemoveAll
		(
			[observer] (IVelocityComponentObserver* other)
			{
				return other == observer;
			}
		);
	}

private:

	friend struct VelocityComponent;
};
