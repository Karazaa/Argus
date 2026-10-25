// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusStaticRecord.h"
#include "SoftPtrLoadStore.h"
#include "HeadlessEntityRecord.generated.h"

UCLASS(BlueprintType)
class ARGUS_API UHeadlessEntityRecord : public UArgusStaticRecord
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FVector m_initialLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	FSoftObjectLoadStore_UArgusEntityTemplate m_entityTemplate;

	void OnAsyncLoaded() const override;
	void ResetSoftPtrLoadStores() override;

#if WITH_EDITOR
private:
	bool UpdateRecord(const FVector& initialLocation, const FSoftObjectLoadStore_UArgusEntityTemplate& entityTemplate);
	friend class UHeadlessEntityComponent;
#endif // WITH_EDITOR
};
