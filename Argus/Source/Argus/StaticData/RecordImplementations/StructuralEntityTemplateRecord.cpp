// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "RecordDefinitions/StructuralEntityTemplateRecord.h"
#include "ArgusEntityTemplate.h"

void UStructuralEntityTemplateRecord::OnAsyncLoaded() const
{
	m_entityTemplate.AsyncPreLoadAndStorePtr([this](const UArgusEntityTemplate* loadedTemplate)
	{
		if (loadedTemplate)
		{
			loadedTemplate->AsyncLoadComponents();
		}
	});
}

void UStructuralEntityTemplateRecord::ResetSoftPtrLoadStores()
{
	m_entityTemplate.ResetHardPtr();
}