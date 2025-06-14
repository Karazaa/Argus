// Copyright Karazaa. This is a part of an RTS project called Argus.
// AUTOGENERATED FILE

#include "ConstructionComponentData.h"
#include "ArgusLogging.h"
#include "ArgusStaticData.h"

void UConstructionComponentData::InstantiateComponentForEntity(ArgusEntity& entity) const
{
	ConstructionComponent* ConstructionComponentRef = entity.AddComponent<ConstructionComponent>();
	ARGUS_RETURN_ON_NULL(ConstructionComponentRef, ArgusECSLog);

	ConstructionComponentRef->m_requiredWorkSeconds = m_requiredWorkSeconds;
	ConstructionComponentRef->m_constructionType = m_constructionType;
}

bool UConstructionComponentData::MatchesType(UComponentData* other) const
{
	if (!other)
	{
		return false;
	}

	const UConstructionComponentData* otherComponentData = Cast<UConstructionComponentData>(other);
	if (!otherComponentData)
	{
		return false;
	}

	return true;
}
