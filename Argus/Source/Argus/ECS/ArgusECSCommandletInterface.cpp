// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusECSCommandletInterface.h"

#if WITH_EDITOR
#include "ArgusEntity.h"
#include "ArgusEntityTemplate.h"
#include "ArgusMacros.h"
#include "ArgusStaticData.h"
#include "RecordDefinitions/StructuralEntityTemplateRecord.h"

void ArgusECSCommandletInterface::InitializeECSForCommandlet()
{
	ArgusEntity::FlushAllEntities();
	ArgusEntity singletonEntity = ArgusEntity::CreateEntity(ArgusECSConstants::k_singletonEntityId);
	singletonEntity.AddComponent<AssetLoadingComponent>();

	const UStructuralEntityTemplateRecord* singletonEntityTemplateRecord = ArgusStaticData::GetRecord<UStructuralEntityTemplateRecord>(1);
	ARGUS_RETURN_ON_NULL(singletonEntityTemplateRecord, ArgusECSLog);
	const UArgusEntityTemplate* singletonEntityTemplate = singletonEntityTemplateRecord->m_entityTemplate.LoadAndStorePtr();
	ARGUS_RETURN_ON_NULL(singletonEntityTemplate, ArgusECSLog);
	singletonEntityTemplate->CacheComponents();
	singletonEntityTemplate->PopulateEntity(singletonEntity);
}

void ArgusECSCommandletInterface::TeardownECSForCommandlet()
{
	ArgusEntity::FlushAllEntities();
}

#endif //WITH_EDITOR
