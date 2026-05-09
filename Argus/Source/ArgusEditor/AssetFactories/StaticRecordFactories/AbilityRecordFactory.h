/// Copyright Karazaa. This is a part of an RTS project called Argus.

#pragma once

#include "RecordDefinitions/AbilityRecord.h"
#include "Developer/AssetTools/Public/AssetTypeActions_Base.h"
#include "Editor/UnrealEd/Classes/Factories/Factory.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "AbilityRecordFactory.generated.h"

#define LOCTEXT_NAMESPACE "AbilityRecordFactory"

UCLASS()
class UAbilityRecordFactory : public UFactory
{
	GENERATED_BODY()

public:
	UAbilityRecordFactory(const FObjectInitializer& objectInitializer);
	virtual UObject* FactoryCreateNew(UClass* classDefinition, UObject* parentObject, FName name, EObjectFlags objectFlags, UObject* context, FFeedbackContext* warning) override;

protected:
	virtual bool IsMacroFactory() const { return false; }
};

class FAssetTypeActions_AbilityRecord : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return LOCTEXT("AbilityRecord", "Ability Record"); }
	virtual uint32 GetCategories() override;
	virtual FColor GetTypeColor() const override { return FColor(79, 239, 174); }

	// COLOR SUGGESTIONS
	// 151, 88, 194 (Archon Purple)
	// 239, 144, 58 (Orange)
	// 79, 239, 174 (Mint)

	virtual FText GetAssetDescription(const FAssetData& assetData) const override { return LOCTEXT("AbilityRecordDesc", "An Ability Record for the Ability Static Database."); }
	virtual UClass* GetSupportedClass() const override { return UAbilityRecord::StaticClass(); }
	virtual const TArray<FText>& GetSubMenus() const override { return m_subMenus; };

private:
	TArray<FText> m_subMenus{ LOCTEXT("StaticDataSubmenu", "Static Records") };
};


#undef LOCTEXT_NAMESPACE