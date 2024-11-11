// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusActorPool.h"

ArgusActorPool::~ArgusActorPool()
{
	ClearPool();
}

AArgusActor* ArgusActorPool::Take(TSoftClassPtr<AArgusActor>& classSoftPointer)
{
	if (!classSoftPointer)
	{
		return nullptr;
	}

	UClass* classPointer = classSoftPointer.LoadSynchronous();

	if (!m_availableObjects.Contains(classPointer))
	{
		// TODO JAMES: Instantiate a new object instance of classPointer and return a pointer to it.
		return nullptr;
	}

	return m_availableObjects[classPointer].Pop();
}

void ArgusActorPool::Release(AArgusActor*& actorPointer)
{
	UClass* classPointer = actorPointer->GetClass();
	


}

void ArgusActorPool::ClearPool()
{

}
