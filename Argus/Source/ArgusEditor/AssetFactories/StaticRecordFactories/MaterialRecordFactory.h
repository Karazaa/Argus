/// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "RecordDefinitions/MaterialRecord.h"
#include "Developer/AssetTools/Public/AssetTypeActions_Base.h"
#include "Editor/UnrealEd/Classes/Factories/Factory.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "MaterialRecordFactory.generated.h"

#define LOCTEXT_NAMESPACE "MaterialRecordFactory"

UCLASS()
class UMaterialRecordFactory : public UFactory
{
	GENERATED_BODY()

public:
	UMaterialRecordFactory(const FObjectInitializer& objectInitializer);
	virtual UObject* FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning) override;

protected:
	virtual bool IsMacroFactory() const { return false; }
};

class FAssetTypeActions_MaterialRecord : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return LOCTEXT("MaterialRecord", "MaterialRecord"); }
	virtual uint32 GetCategories() override;
	virtual FColor GetTypeColor() const override { return FColor(79, 239, 174); }

	virtual FText GetAssetDescription(const FAssetData& assetData) const override { return LOCTEXT("MaterialRecordDesc", "An MaterialRecord for the MaterialRecord Static Database."); }
	virtual UClass* GetSupportedClass() const override { return UMaterialRecord::StaticClass(); }
	virtual const TArray<FText>& GetSubMenus() const override { return m_subMenus; };

private:
	TArray<FText> m_subMenus{ LOCTEXT("StaticDataSubmenu", "Static Records") };
};


#undef LOCTEXT_NAMESPACE
