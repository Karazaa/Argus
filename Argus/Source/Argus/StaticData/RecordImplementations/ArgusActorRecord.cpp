// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "RecordDefinitions/ArgusActorRecord.h"
#include "ArgusStaticData.h"

void UArgusActorRecord::OnAsyncLoaded() const
{
	m_argusActorClass.AsyncPreLoadAndStorePtr();
	m_entityTemplate.AsyncPreLoadAndStorePtr();
	m_actorInfoIcon.AsyncPreLoadAndStorePtr();
}