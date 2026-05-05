// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "RecordDefinitions/MaterialRecord.h"
#include "ArgusStaticData.h"

void UMaterialRecord::OnAsyncLoaded() const
{
	m_material.AsyncPreLoadAndStorePtr();
}

void UMaterialRecord::ResetSoftPtrLoadStores()
{
	m_material.ResetHardPtr();
}
