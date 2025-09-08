// Copyright (c) Bruno Pereira Costa

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LidarPointCloud.h"
#include "LidarPointCloudShared.h"
#include "LidarPointCloudSettings.h"
#include "LidarPointCloudComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "LabellingDataManager.generated.h"

USTRUCT(BlueprintType)
struct FLidarPointCloudLabelled
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FLidarPointCloudPoint> Points;

	int32 ClassificationID;
	FLinearColor LabelColor;
};

UCLASS()
class SPHERIACTUS_API ULabellingDataManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Exports a Point Cloud to given file. Slower than ExportLabelledPointCloudComponent due to copying all points. */
	UFUNCTION(BlueprintCallable, Category = "Labelling Data Manager")
	static void ExportLabelledPointCloud(ULidarPointCloud* LabelledPointCloud, const TArray<FString>& LabelNames, const FString& FilePath);

	UFUNCTION(BlueprintCallable, Category = "Labelling Data Manager")
	static void ExportLabelledPointCloudComponent(const ULidarPointCloudComponent* LabelledPointCloudComponent, const TArray<FString>& LabelNames, const FString& FilePath, const FString& OriginalFilePath);
};