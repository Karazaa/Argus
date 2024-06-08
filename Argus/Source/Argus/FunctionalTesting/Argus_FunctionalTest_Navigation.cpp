// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "Argus_FunctionalTest_Navigation.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

bool AArgus_FunctionalTest_Navigation::DidArgusFunctionalTestFail()
{
	if (!m_argusActor.IsValid())
	{
		m_testFailedMessage = TEXT("Failed test due to invalid setup. m_argusActor must be a valid reference.");
		return true;
	}

	if (!m_goalActor.IsValid())
	{
		m_testFailedMessage = TEXT("Failed test due to invalid setup. m_goalActor must be a valid reference.");
		return true;
	}

	ArgusEntity argusEntity = m_argusActor->GetEntity();
	if (!argusEntity)
	{
		m_testFailedMessage = TEXT("Failed test due to ArgusActor not having a valid backing ArgusEntity.");
		return true;
	}

	TaskComponent* taskComponent = argusEntity.GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		m_testFailedMessage = TEXT("Failed test due to ArgusActor not having a valid backing Task Component.");
		return true;
	}

	TargetingComponent* targetingComponent = argusEntity.GetComponent<TargetingComponent>();
	if (!targetingComponent)
	{
		m_testFailedMessage = TEXT("Failed test due to ArgusActor not having a valid backing Targeting Component.");
		return true;
	}

	return false;
}

bool AArgus_FunctionalTest_Navigation::DidCurrentTestStepSucceed()
{
	switch (m_testStepIndex)
	{
		case 0:
			return DidNavigationToLocationStepSucceed();
		case 1:
			return DidNavigationToEntityStepSucceed();
		default:
			break;
	}
	return false;
}

void AArgus_FunctionalTest_Navigation::StartNextTestStep()
{
	switch (m_testStepIndex)
	{
		case 0:
			StartNavigationToLocationStep();
			break;
		case 1:
			StartNavigationToEntityStep();
			break;
		default:
			break;
	}
}

void AArgus_FunctionalTest_Navigation::StartNavigationToLocationStep()
{
	StartStep(TEXT("Navigate to Location"));

	if (DidArgusFunctionalTestFail())
	{
		ConcludeFailedArgusFunctionalTest();
		return;
	}

	ArgusEntity argusEntity = m_argusActor->GetEntity();
	TaskComponent* taskComponent = argusEntity.GetComponent<TaskComponent>();
	TargetingComponent* targetingComponent = argusEntity.GetComponent<TargetingComponent>();

	if (taskComponent && targetingComponent)
	{
		targetingComponent->m_targetLocation = m_goalActor->GetActorLocation();
		taskComponent->m_currentTask = ETask::ProcessMoveToLocationCommand;
	}
	else
	{
		m_testFailedMessage = TEXT("Failed test due to invalid setup. Missing ArgusEntity Componentns.");
		ConcludeFailedArgusFunctionalTest();
	}
}

void AArgus_FunctionalTest_Navigation::StartNavigationToEntityStep()
{
	FinishStep();
	StartStep(TEXT("Navigate to Entity"));
}

bool AArgus_FunctionalTest_Navigation::DidNavigationToLocationStepSucceed()
{
	if (FVector::DistSquared(m_argusActor->GetActorLocation(), m_goalActor->GetActorLocation()) < FMath::Square(m_successDistance))
	{
		m_testSucceededMessage = TEXT("Argus Actor successfully reached goal target!");
		return true;
	}

	return false;
}

bool AArgus_FunctionalTest_Navigation::DidNavigationToEntityStepSucceed()
{
	return true;
}