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
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. %s must be a valid reference."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_argusActor)
		);
		return true;
	}

	if (!m_goalLocationActor.IsValid())
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. %s must be a valid reference."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_goalLocationActor)
		);
		return true;
	}

	if (!m_goalEntityActor.IsValid())
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. %s must be a valid reference."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_goalEntityActor)
		);
		return true;
	}

	ArgusEntity argusEntity = m_argusActor->GetEntity();
	if (!argusEntity)
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to %s not having a valid backing %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_argusActor),
			ARGUS_NAMEOF(ArgusEntity)
		);
		return true;
	}

	TaskComponent* taskComponent = argusEntity.GetComponent<TaskComponent>();
	if (!taskComponent)
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to %s not having a valid backing %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(argusEntity),
			ARGUS_NAMEOF(TaskComponent)
		);
		return true;
	}

	TargetingComponent* targetingComponent = argusEntity.GetComponent<TargetingComponent>();
	if (!targetingComponent)
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to %s not having a valid backing %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(argusEntity),
			ARGUS_NAMEOF(TargetingComponent)
		);
		return true;
	}

	return false;
}

bool ANavigationFunctionalTest::DidCurrentTestStepSucceed()
{
	switch (m_testStepIndex)
	{
		case 0:
			return DidNavigationToLocationTestStepSucceed();
		case 1:
			return DidNavigationToEntityTestStepSucceed();
		case 2:
			return DidNavigationThroughWaypointsTestStepSucceed();
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
			StartNavigationToLocationTestStep();
			break;
		case 1:
			StartNavigationToEntityTestStep();
			break;
		case 2:
			StartNavigationThroughWaypointsTestStep();
			break;
		default:
			break;
	}
}

void ANavigationFunctionalTest::StartNavigationToLocationTestStep()
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
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. Missing %s Components."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity)
		);
		ConcludeFailedArgusFunctionalTest();
	}
}

void ANavigationFunctionalTest::StartNavigationToEntityTestStep()
{
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
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to invalid setup. Missing %s Components."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusEntity)
		);
		ConcludeFailedArgusFunctionalTest();
	}
}

void ANavigationFunctionalTest::StartNavigationThroughWaypointsTestStep()
{
	StartStep(TEXT("Navigate through waypoints."));
}

bool ANavigationFunctionalTest::DidNavigationToLocationTestStepSucceed()
{
	if (FVector::DistSquared(m_argusActor->GetActorLocation(), m_goalLocationActor->GetActorLocation()) < FMath::Square(m_successDistance))
	{
		return true;
	}

	return false;
}

bool ANavigationFunctionalTest::DidNavigationToEntityTestStepSucceed()
{
	if (FVector::DistSquared(m_argusActor->GetActorLocation(), m_goalEntityActor->GetActorLocation()) < FMath::Square(m_successDistance))
	{
		m_testSucceededMessage = FString::Printf
		(
			TEXT("[%s] %s successfully reached goal targets!"),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(m_argusActor)
		);
		return true;
	}

	return false;
}

bool ANavigationFunctionalTest::DidNavigationThroughWaypointsTestStepSucceed()
{
	return true;
}