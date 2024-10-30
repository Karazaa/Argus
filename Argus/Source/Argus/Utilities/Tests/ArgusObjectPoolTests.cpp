// Copyright Karazaa. This is a part of an RTS project called Argus.

#include "ArgusMacros.h"
#include "ArgusObjectPool.h"
#include "Misc/AutomationTest.h"

struct TestPoolable : public IObjectPoolable
{
	float m_dummyValue = 0.0f;

	virtual void Reset() override
	{
		m_dummyValue = 0.0f;
	}
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusObjectPoolTakeTest, "Argus.Utilities.ArgusObjectPool.TakeTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusObjectPoolTakeTest::RunTest(const FString& Parameters)
{
	ArgusObjectPool<TestPoolable> objectPool;
	int numAvailableObjects = objectPool.GetNumAvailableObjects();

#pragma region Test that there are no available objects in the object pool initially.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, and testing that there are no available objects in it to begin with."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusObjectPool)
		),
		numAvailableObjects,
		0
	);
#pragma endregion

	TestPoolable* testPoolable = objectPool.Take();
	numAvailableObjects = objectPool.GetNumAvailableObjects();

#pragma region Test that there are no available objects in the object pool after a take.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, calling %s, and testing that there are no available objects in the pool after."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusObjectPool),
			ARGUS_NAMEOF(ArgusObjectPool::Take)
		),
		numAvailableObjects,
		0
	);
#pragma endregion

#pragma region Test that the taken object from the pool is not null.
	TestTrue
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, calling %s, and testing that the returned pointer is not null."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusObjectPool),
			ARGUS_NAMEOF(ArgusObjectPool::Take)
		),
		testPoolable != nullptr
	);
#pragma endregion

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ArgusUtilitiesArgusObjectPoolReleaseTest, "Argus.Utilities.ArgusObjectPool.ReleaseTest", EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::SmokeFilter)
bool ArgusUtilitiesArgusObjectPoolReleaseTest::RunTest(const FString& Parameters)
{
	ArgusObjectPool<TestPoolable> objectPool;

	objectPool.Release(nullptr);
	int numAvailableObjects = objectPool.GetNumAvailableObjects();

#pragma region Test that there are no available objects in the object pool after a fake release.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, calling %s on null, and testing that there are no available objects in the pool after."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusObjectPool),
			ARGUS_NAMEOF(ArgusObjectPool::Release)
		),
		numAvailableObjects,
		0
	);
#pragma endregion

	TestPoolable* testPoolable = objectPool.Take();
	numAvailableObjects = objectPool.GetNumAvailableObjects();

#pragma region Test that there are no available objects in the object pool after a take.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, calling %s, and testing that there are no available objects in the pool after."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusObjectPool),
			ARGUS_NAMEOF(ArgusObjectPool::Take)
		),
		numAvailableObjects,
		0
	);
#pragma endregion

	objectPool.Release(testPoolable);
	numAvailableObjects = objectPool.GetNumAvailableObjects();

#pragma region Test that there are no available objects in the object pool after a take.
	TestEqual
	(
		FString::Printf
		(
			TEXT("[%s] Creating a %s, calling %s and then %s, and testing that there is one available object in the pool after."),
			ARGUS_FUNCNAME,
			ARGUS_NAMEOF(ArgusObjectPool),
			ARGUS_NAMEOF(ArgusObjectPool::Take),
			ARGUS_NAMEOF(ArgusObjectPool::Release)
		),
		numAvailableObjects,
		1
	);
#pragma endregion

	return true;
}

#if WITH_AUTOMATION_TESTS
#endif //WITH_AUTOMATION_TESTS