// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "RecordDefinitions/ArgusActorRecord.h"
#include "ArgusStaticData.h"

void UArgusActorRecord::OnAsyncLoaded() const
{
	m_argusActorClass.AsyncPreLoadAndStorePtr();
	m_entityTemplate.AsyncPreLoadAndStorePtr([this](const UArgusEntityTemplate* loadedTemplate)
	{
	});
	m_actorInfoIcon.AsyncPreLoadAndStorePtr();
}