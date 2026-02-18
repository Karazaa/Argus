// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "CoreMinimal.h"
#include "ComponentDependencies/ResourceSet.h"

UENUM()
enum class EEntityCategoryType : uint8
{
	Extractor,
	Carrier,
	ResourceSink,
	ResourceSource,
	Count
};

struct EntityCategory
{
	EEntityCategoryType m_entityCategoryType = EEntityCategoryType::Count;
	EResourceType m_resourceType = EResourceType::Count;
};