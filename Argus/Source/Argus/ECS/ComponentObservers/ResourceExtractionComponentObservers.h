// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#pragma once

#include "CoreMinimal.h"
#include "ComponentDependencies/ResourceExtractionComponentStates.h"

class IResourceExtractionComponentObserver
{
public:
};

class ResourceExtractionComponentObservers
{
private:
	TArray<IResourceExtractionComponentObserver*> m_ResourceExtractionComponentObservers;

public:
	void AddObserver(IResourceExtractionComponentObserver* observer)
	{
		if (!observer)
		{
			return;
		}

		m_ResourceExtractionComponentObservers.Add(observer);
	}

	void RemoveObserver(IResourceExtractionComponentObserver* observer)
	{
		if (!observer)
		{
			return;
		}

		m_ResourceExtractionComponentObservers.RemoveAll
		(
			[observer] (IResourceExtractionComponentObserver* other)
			{
				return other == observer;
			}
		);
	}

private:

	friend struct ResourceExtractionComponent;
};
