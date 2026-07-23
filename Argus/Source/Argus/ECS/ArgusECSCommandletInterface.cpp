// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusECSCommandletInterface.h"

#if WITH_EDITOR
#include "ArgusEntity.h"

void ArgusECSCommandletInterface::InitializeECSForCommandlet()
{
	ArgusEntity::FlushAllEntities();
	ArgusEntity singletonEntity = ArgusEntity::CreateEntity(ArgusECSConstants::k_singletonEntityId);
	singletonEntity.AddComponent<AssetLoadingComponent>();
}

void ArgusECSCommandletInterface::TeardownECSForCommandlet()
{
	ArgusEntity::FlushAllEntities();
}

#endif //WITH_EDITOR
