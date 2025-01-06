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

	NavigationComponent* navigationComponent = argusEntity.GetComponent<NavigationComponent>();
	if (!navigationComponent)
	{
		m_testFailedMessage = FString::Printf
		(
			TEXT("[%s] Failed test due to %s not having a valid backing %s."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(argusEntity),
			ARGUS_NAMEOF(navigationComponent)
		);
		return true;
	}

	return false;
}

bool ANavigationFunctionalTest::DidCurrentTestStepSucceed()
{
	switch (m_testStepIndex)
	{
		case 0u:
			return DidNavigationToLocationTestStepSucceed();
		case 1u:
			return DidNavigationToEntityTestStepSucceed();
		case 2u:
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
		case 0u:
			StartNavigationToLocationTestStep();
			break;
		case 1u:
			StartNavigationToEntityTestStep();
			break;
		case 2u:
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
		taskComponent->m_movementState = MovementState::ProcessMoveToLocationCommand;
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
		goalTaskComponent->m_movementState = MovementState::ProcessMoveToLocationCommand;
		targetingComponent->m_targetEntityId = goalEntity.GetId();
		taskComponent->m_movementState = MovementState::ProcessMoveToEntityCommand;
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
	const float waypointDistanceOffset = 300.0f;

	StartStep(TEXT("Navigate through waypoints."));

	if (DidArgusFunctionalTestFail())
	{
		ConcludeFailedArgusFunctionalTest();
		return;
	}

	FVector location = m_goalLocationActor->GetActorLocation();
	m_waypoint0 = location;
	m_waypoint1 = location;
	m_waypoint2 = location;

	m_waypoint0.X += waypointDistanceOffset;
	m_waypoint1.Y += waypointDistanceOffset;
	m_waypoint2.X -= waypointDistanceOffset;

	ArgusEntity argusEntity = m_argusActor->GetEntity();
	TaskComponent* taskComponent = argusEntity.GetComponent<TaskComponent>();
	TargetingComponent* targetingComponent = argusEntity.GetComponent<TargetingComponent>();
	NavigationComponent* navigationComponent = argusEntity.GetComponent<NavigationComponent>();

	if (taskComponent && targetingComponent && navigationComponent)
	{
		targetingComponent->m_targetLocation = m_waypoint0;
		navigationComponent->m_queuedWaypoints.push(m_waypoint1);
		navigationComponent->m_queuedWaypoints.push(m_waypoint2);
		taskComponent->m_movementState = MovementState::ProcessMoveToLocationCommand;
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
	if (FVector::DistSquared(m_argusActor->GetActorLocation(), m_goalEntityActor->GetActorLocation()) < FMath::Square(400.0f))
	{
		return true;
	}

	return false;
}

bool ANavigationFunctionalTest::DidNavigationThroughWaypointsTestStepSucceed()
{
	if (FVector::DistSquared(m_argusActor->GetActorLocation(), m_waypoint2) < FMath::Square(m_successDistance))
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