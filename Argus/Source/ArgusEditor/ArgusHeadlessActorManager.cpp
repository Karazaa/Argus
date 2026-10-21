// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusHeadlessActorManager.h"
#include "ArgusHeadlessActor.h"
#include "Editor.h"
#include "Engine.h"
#include "GameFramework/Actor.h"

void ArgusHeadlessActorManager::Initialize()
{
	if (!GEngine)
	{
		return;
	}

	GEngine->OnLevelActorAdded().AddRaw(this, &ArgusHeadlessActorManager::OnLevelActorAdded);
	FEditorDelegates::OnDuplicateActorsBegin.AddRaw(this, &ArgusHeadlessActorManager::OnDuplicateActorsBegin);
	FEditorDelegates::OnDuplicateActorsEnd.AddRaw(this, &ArgusHeadlessActorManager::OnDuplicateActorsEnd);
}

void ArgusHeadlessActorManager::Teardown()
{
	FEditorDelegates::OnDuplicateActorsBegin.RemoveAll(this);
	FEditorDelegates::OnDuplicateActorsEnd.RemoveAll(this);

	if (!GEngine)
	{
		return;
	}

	GEngine->OnLevelActorAdded().RemoveAll(this);
}

void ArgusHeadlessActorManager::OnLevelActorAdded(AActor* actor)
{
	AArgusHeadlessActor* argusHeadlessActor = Cast<AArgusHeadlessActor>(actor);
	if (!argusHeadlessActor || argusHeadlessActor->HasAnyFlags(RF_Transient | RF_WasLoaded))
	{
		return;
	}

	if (m_isDuplicatingActors)
	{
		// If we are duplicating actors, we don't even want to try fixing up the HeadlessActorRecords until after finishing duplication. 
		// Properties from the source actor have not yet been copied over.

		// TODO JAMES: Queue request

		return;
	}

	// TODO JAMES: Process request
}

void ArgusHeadlessActorManager::OnDuplicateActorsBegin()
{
	m_isDuplicatingActors = true;
}

void ArgusHeadlessActorManager::OnDuplicateActorsEnd()
{
	m_isDuplicatingActors = false;

	// TODO JAMES: Process request and empty queue
}