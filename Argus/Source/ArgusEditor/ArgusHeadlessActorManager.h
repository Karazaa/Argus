// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"

class AActor;

class ArgusHeadlessActorManager
{
public:
	void Initialize();
	void Teardown();
private:
	void OnLevelActorAdded(AActor* actor);
	void OnDuplicateActorsBegin();
	void OnDuplicateActorsEnd();

	bool m_isDuplicatingActors = false;
};