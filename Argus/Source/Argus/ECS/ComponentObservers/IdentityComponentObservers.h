// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "CoreMinimal.h"
#include "ComponentDependencies/IdentityComponentStates.h"

class IIdentityComponentObserver
{
public:
};

class IdentityComponentObservers
{
private:
	TArray<IIdentityComponentObserver*> m_IdentityComponentObservers;

public:
	void AddObserver(IIdentityComponentObserver* observer)
	{
		if (!observer)
		{
			return;
		}

		m_IdentityComponentObservers.Add(observer);
	}

	void RemoveObserver(IIdentityComponentObserver* observer)
	{
		if (!observer)
		{
			return;
		}

		m_IdentityComponentObservers.RemoveAll
		(
			[observer] (IIdentityComponentObserver* other)
			{
				return other == observer;
			}
		);
	}

private:

	friend struct IdentityComponent;
};
