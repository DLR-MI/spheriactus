// Copyright (c) Bruno Pereira Costa

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "LidarPointCloudShared.h"
#include "LidarPointCloudComponent.h"
#include "Containers/Array.h"
#include "Async/ParallelFor.h"
#include "Math/UnrealMathUtility.h"
#include "SphereTraceCascade.generated.h"

UCLASS()
class SPHERIACTUS_API USphereTraceCascade : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Sphere Trace Cascade")
    static void SphereTraceCascade(
        bool bLabel,
        int32 LabelID,
        int32 NumberOfLayers,
        float Radius,
        FVector AimingLocation,
        FVector AimingDirection,
        FLinearColor CurrentLabelColor,
        TArray<AActor*> ActorsToIgnore,
        TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes,
        ULidarPointCloudComponent* OriginalPointCloudComponent,
        ULidarPointCloudComponent* TargetedPointCloudComponent,
        UStaticMeshComponent* CylinderMesh,
        TArray<FVector>& SphereCenters,
        bool& FirstTraceBlocked
    );

private:
    static void SphereTrace(
        bool bLabel,
        int32 LabelID,
        FVector FirstImpactPoint,
        FVector StartLocation,
        FVector EndLocation,
        float Radius,
        FLinearColor CurrentLabelColor,
        TArray<AActor*> ActorsToIgnore,
        TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes,
        ULidarPointCloudComponent* OriginalPointCloudComponent,
        TArray<FLidarPointCloudPoint*>& LocalTargetedPoints,
        TArray<FVector>& LocalSphereCenters,
        bool DoTrace,
        FVector AimingLocation,
        FVector AimingDirection);

    static void UpdateCylinderTransform(
        UStaticMeshComponent* CylinderMesh,
        FVector& Start,
        FVector& End,
        FVector& Direction,
        float Radius,
        bool bHit);
};