// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Views/ArgusActorInfoView.h"
#include "ArgusEntity.h"

void UArgusActorInfoView::SetInitialDisplay(ArgusEntity argusEntity)
{
	if (!argusEntity)
	{
		return;
	}

	for (int32 i = 0; i < m_childArgusActorWidgets.Num(); ++i)
	{
		if (!m_childArgusActorWidgets[i])
		{
			continue;
		}

		m_childArgusActorWidgets[i]->SetInitialDisplay(argusEntity);
	}
}

void UArgusActorInfoView::RefreshDisplay(ArgusEntity argusEntity)
{
	ARGUS_TRACE(UArgusActorInfoView::RefreshDisplay);

	if (!argusEntity)
	{
		return;
	}

	for (int32 i = 0; i < m_childArgusActorWidgets.Num(); ++i)
	{
		if (!m_childArgusActorWidgets[i])
		{
			continue;
		}

		m_childArgusActorWidgets[i]->RefreshDisplay(argusEntity);
	}
}