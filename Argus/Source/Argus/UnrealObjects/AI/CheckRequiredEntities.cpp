// Copyright Karazaa. This is a part of an RTS project called Argus.



#include "CheckRequiredEntities.h"

void UCheckRequiredEntities::OnArgusStartTask()
{
	bool bSuccss = false;
	if (m_aiController)
	{
		bSuccss = m_aiController->HasRequiredEntities();
	}
	FinishTask(bSuccss);
}
