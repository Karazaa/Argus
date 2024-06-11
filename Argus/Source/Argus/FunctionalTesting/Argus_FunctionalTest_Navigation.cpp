// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "NavigationFunctionalTest.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ANavigationFunctionalTest::ANavigationFunctionalTest(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	TestLabel = k_testName;
}

bool ANavigationFunctionalTest::DidArgusFunctionalTestFail()
{
	if (!m_argusActor.IsValid())
	{
		m_testFailedMessage = TEXT("Failed test due to invalid setup. m_argusActor must be a valid reference.");
		return true;
	}

	if (!m_goalLocationActor.IsValid())
	{
		m_testFailedMessage = TEXT("Failed test due to invalid setup. m_goalLocationActor must be a valid reference.");
		return true;
	}

	if (!m_goalEntityActor.IsValid())
	{
		m_testFailedMessage = TEXT("Failed test due to invalid setup. m_goalEntityActor must be a valid reference.");
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

bool ANavigationFunctionalTest::DidCurrentTestStepSucceed()
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

void ANavigationFunctionalTest::StartNextTestStep()
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

void ANavigationFunctionalTest::StartNavigationToLocationStep()
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
		targetingComponent->m_targetLocation = m_goalLocationActor->GetActorLocation();
		taskComponent->m_currentTask = ETask::ProcessMoveToLocationCommand;
	}
	else
	{
		m_testFailedMessage = TEXT("Failed test due to invalid setup. Missing ArgusEntity Components.");
		ConcludeFailedArgusFunctionalTest();
	}
}

void ANavigationFunctionalTest::StartNavigationToEntityStep()
{
	FinishStep();
	StartStep(TEXT("Navigate to Entity"));

	if (DidArgusFunctionalTestFail())
	{
		ConcludeFailedArgusFunctionalTest();
		return;
	}

	ArgusEntity argusEntity = m_argusActor->GetEntity();
	TaskComponent* taskComponent = argusEntity.GetComponent<TaskComponent>();
	TargetingComponent* targetingComponent = argusEntity.GetComponent<TargetingComponent>();
	ArgusEntity goalEntity = m_goalEntityActor->GetEntity();
	TaskComponent* goalTaskComponent = goalEntity.GetComponent<TaskComponent>();
	TargetingComponent* goalTargetingComponent = goalEntity.GetComponent<TargetingComponent>();

	if (taskComponent && targetingComponent && goalTaskComponent && goalTargetingComponent)
	{
		goalTargetingComponent->m_targetLocation = m_goalLocationActor->GetActorLocation();
		goalTaskComponent->m_currentTask = ETask::ProcessMoveToLocationCommand;
		targetingComponent->m_targetEntityId = goalEntity.GetId();
		taskComponent->m_currentTask = ETask::ProcessMoveToEntityCommand;
	}
	else
	{
		m_testFailedMessage = TEXT("Failed test due to invalid setup. Missing ArgusEntity Components.");
		ConcludeFailedArgusFunctionalTest();
	}
}

bool ANavigationFunctionalTest::DidNavigationToLocationStepSucceed()
{
	if (FVector::DistSquared(m_argusActor->GetActorLocation(), m_goalLocationActor->GetActorLocation()) < FMath::Square(m_successDistance))
	{
		return true;
	}

	return false;
}

bool ANavigationFunctionalTest::DidNavigationToEntityStepSucceed()
{
	if (FVector::DistSquared(m_argusActor->GetActorLocation(), m_goalEntityActor->GetActorLocation()) < FMath::Square(m_successDistance))
	{
		m_testSucceededMessage = TEXT("Argus Actor successfully reached goal targets!");
		return true;
	}

	return false;
}