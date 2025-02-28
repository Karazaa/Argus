// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "Blueprint/UserWidget.h"
#include "ComponentDependencies/ResourceSet.h"
#include "Misc/Optional.h"
#include "ResourceWidget.generated.h"

UCLASS()
class UResourceWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateDisplay(EResourceType resourceType, int32 quantity);

private:
	void ChangeResourceType(EResourceType newResourceType);
	void ChangeResourceQuantity(int32 newQuantity);

protected:
	TOptional<EResourceType> m_currentResourceType = NullOpt;
	TOptional<int32> m_currentResourceQuantity = NullOpt;
};