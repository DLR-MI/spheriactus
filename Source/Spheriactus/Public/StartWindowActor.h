// Copyright (c) Bruno Pereira Costa

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SStartWidgetSwitcher.h"
#include "StartWindowActor.generated.h"


UCLASS()
class SPHERIACTUS_API AStartWindowActor : public AActor
{
	GENERATED_BODY()
	
public:
	AStartWindowActor();

	UFUNCTION(BlueprintCallable, Category = "OpenStartWindow")
	void OpenStartWindow(const FString& DefaultPath, FString& OutFileName);

	UFUNCTION(BlueprintCallable, Category = "OpenStartWindow")
	TMap<FString, FLinearColor> GetLabelEntries() const;

	UFUNCTION(BlueprintCallable, Category = "OpenStartWindow")
	TMap<int32, FLinearColor> GetClassificationColors() const;

	UFUNCTION(BlueprintCallable, Category = "OpenStartWindow")
	FString GetExportFilePath() const;

	UFUNCTION(BlueprintCallable, Category = "OpenStartWindow")
	FString GetImportFilePath() const;

private:
	TSharedPtr<SStartWidgetSwitcher> StartWidgetSwitcher;
	TSharedPtr<SWindow> StartWindow;
	FString SelectedFilePath;
};