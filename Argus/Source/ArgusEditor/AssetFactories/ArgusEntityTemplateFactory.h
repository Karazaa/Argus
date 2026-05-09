// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "ArgusEntityTemplate.h"
#include "Developer/AssetTools/Public/AssetTypeActions_Base.h"
#include "Editor/UnrealEd/Classes/Factories/Factory.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "ArgusEntityTemplateFactory.generated.h"

#define LOCTEXT_NAMESPACE "ArgusEntityTemplateFactory"

UCLASS()
class UArgusEntityTemplateFactory : public UFactory
{
	GENERATED_BODY()

public:
	UArgusEntityTemplateFactory(const FObjectInitializer& objectInitializer);
	virtual UObject* FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning) override;

protected:
	virtual bool IsMacroFactory() const { return false; }
};

class FAssetTypeActions_ArgusEntityTemplate : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return LOCTEXT("ArgusEntityTemplate", "Argus Entity Template"); }
	virtual uint32 GetCategories() override;
	virtual FColor GetTypeColor() const override { return FColor(151, 88, 194); }

	// COLOR SUGGESTIONS
	// 151, 88, 194 (Archon Purple)
	// 239, 144, 58 (Orange)
	// 79, 239, 174 (Mint)

	virtual FText GetAssetDescription(const FAssetData& assetData) const override { return LOCTEXT("ArgusEntityTemplateDesc", "Data Asset that defines which components should live on an ArgusEntity."); }
	virtual UClass* GetSupportedClass() const override { return UArgusEntityTemplate::StaticClass(); }
};


#undef LOCTEXT_NAMESPACE