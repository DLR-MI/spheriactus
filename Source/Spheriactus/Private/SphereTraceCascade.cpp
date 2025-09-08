// Copyright (c) Bruno Pereira Costa

#include "SphereTraceCascade.h"

// Converts axial coordinates (q, r) to local 2D coordinates (pointy-topped).
FVector2D AxialToLocal2D(const FIntPoint& axial, float hexSize)
{
    float sqrt3 = FMath::Sqrt(3.0f);
    float localX = hexSize * (sqrt3 * axial.X + (sqrt3 / 2.0f) * axial.Y);
    float localY = hexSize * (1.5f * axial.Y);
    return FVector2D(localX, localY);
}

void USphereTraceCascade::SphereTraceCascade(
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
    bool& FirstTraceBlocked)
{

    if (!GEngine->GameViewport->GetWorld())
    {
        return;
    }

    if (!OriginalPointCloudComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Point Cloud not loaded or detected"));
        return;
    }

    // Do first sphere trace to check if there are obstacles so that we do more traces or stop
    FVector FirstEndTrace = AimingLocation + (AimingDirection * 10000.0);
    FHitResult FirstHitResult;

    bool bFirstHit = UKismetSystemLibrary::SphereTraceSingleForObjects(
        OriginalPointCloudComponent->GetWorld(),
        AimingLocation,
        FirstEndTrace,
        Radius,
        ObjectTypes,
        true,
        ActorsToIgnore,
        EDrawDebugTrace::None,
        FirstHitResult,
        true,
        FLinearColor::Red,
        FLinearColor::Green,
        5.0f
    );


    if (bFirstHit)
    {
        FVector FirstImpactPoint = FirstHitResult.ImpactPoint;
        UpdateCylinderTransform(CylinderMesh, AimingLocation, FirstImpactPoint, AimingDirection, 0.001, true);

        TArray<FVector> TraceStart;
        TArray<FVector> TraceEnd;
        TArray<bool> DoTrace;

        // Pre-allocate memory
        int32 MaxTotalTraces = (NumberOfLayers * (NumberOfLayers + 1) * 3) + 1;
        TraceStart.Reserve(MaxTotalTraces);
        TraceEnd.Reserve(MaxTotalTraces);
        DoTrace.Reserve(MaxTotalTraces);

        // Conditions for first sphere trace
        TraceStart.Add(AimingLocation);
        TraceEnd.Add(FirstEndTrace);
        DoTrace.Add(false); // used to prevent repetition of first sphere trace

        if (NumberOfLayers > 0)
        {
            // Compute normalized aiming direction.
            FVector aimingDirNorm = AimingDirection.GetSafeNormal(0.0001);

            // Determine an axis in the hex grid plane.
            FVector axisX = FVector::CrossProduct(FVector(aimingDirNorm.X + 1, 0, 0), aimingDirNorm).GetSafeNormal(0.0001);

            // Create axisY as perpendicular to both aimingDirNorm and axisX.
            FVector axisY = FVector::CrossProduct(aimingDirNorm, axisX).GetSafeNormal(0.0001);

            // Set the grid origin with offset of 2.1 x radius x AimingDirection
            // Offset increases sphere projection stability as the sphere traces start far away from first hit neighbouring points, on average
            FVector gridOrigin = FirstImpactPoint - aimingDirNorm * 2.1 * Radius;

            // Set hexSize. Scale factor of 2/sqrt(3) corrects for no overlapping
            float hexSize = 2 / FMath::Sqrt(3.0f) * Radius;

            // Loop for each layer (ring) around the center.
            // Each layer is a “ring” of hex cells. The center (layer 0) is already added.
            for (int32 layer = 1; layer <= NumberOfLayers; ++layer)
            {
                // Start at axial coordinate (layer, 0).
                FIntPoint axial(layer, 0);

                for (int side = 0; side < 6; ++side)
                {
                    // Each side has "layer" steps.
                    for (int step = 0; step < layer; ++step)
                    {
                        // Convert axial coordinates to a 2D local position.
                        FVector2D localPos = AxialToLocal2D(axial, hexSize);

                        // Map local 2D coordinate (localPos.X, localPos.Y) into 3D world space.
                        // We assume that axisX maps to the local X direction and axisY maps to the local Y.
                        FVector worldPoint = gridOrigin + (axisX * localPos.X) + (axisY * localPos.Y);
                        TraceStart.Add(worldPoint);

                        // Calculate a trace end.
                        float vectorLength = (worldPoint - FirstImpactPoint).Size();
                        // Offset of 2.6 x radius x aimingDirNorm added so that a sphere travels/penetrates half a radius distance and then trace ends
                        TraceEnd.Add(worldPoint + aimingDirNorm * Radius * 2.6);
                        //TraceEnd.Add(aimingDirNorm * Radius * 1.5 + FirstImpactPoint);
                        DoTrace.Add(true);

                        // Move to the next axial coordinate along this side.
                        FIntPoint axialDir;
                        switch (side)
                        {
                        case 0: axialDir = FIntPoint(-1, +1); break;
                        case 1: axialDir = FIntPoint(-1, 0); break;
                        case 2: axialDir = FIntPoint(0, -1); break;
                        case 3: axialDir = FIntPoint(+1, -1); break;
                        case 4: axialDir = FIntPoint(+1, 0); break;
                        case 5: axialDir = FIntPoint(0, +1); break;
                        default: axialDir = FIntPoint(0, 0);  break;
                        }
                        axial.X += axialDir.X;
                        axial.Y += axialDir.Y;
                    }
                }
            }
        }

        // Create thread-local containers
        TArray<TArray<FLidarPointCloudPoint*>> ThreadLocalTargetedPoints;
        TArray<TArray<FVector>> ThreadLocalSphereCenters;
        ThreadLocalTargetedPoints.AddDefaulted(TraceStart.Num());
        ThreadLocalSphereCenters.AddDefaulted(TraceStart.Num());

        // Perform sphere traces in parallel
        ParallelFor(TraceStart.Num(), [&](int32 Index)
            {
                SphereTrace(
                    bLabel,
                    LabelID,
                    FirstImpactPoint,
                    TraceStart[Index],
                    TraceEnd[Index],
                    Radius,
                    CurrentLabelColor,
                    ActorsToIgnore,
                    ObjectTypes,
                    OriginalPointCloudComponent,
                    ThreadLocalTargetedPoints[Index],
                    ThreadLocalSphereCenters[Index],
                    DoTrace[Index],
                    AimingLocation,
                    AimingDirection);
            }
        );


        // Combine local arrays into main arrays
        TArray<FLidarPointCloudPoint*> TargetedPoints;
        for (int32 Index = 0; Index < TraceStart.Num(); ++Index)
        {
            for (FLidarPointCloudPoint*& Point : ThreadLocalTargetedPoints[Index]) // iterating in fact pointers
            {
                TargetedPoints.Add(Point);
            }
            for (const FVector& Center : ThreadLocalSphereCenters[Index])
            {
                SphereCenters.AddUnique(Center);
            }
        }
        TargetedPointCloudComponent->GetPointCloud()->Octree.Empty(false);
        if (TargetedPoints.Num() > 0)
        {
            TargetedPointCloudComponent->GetPointCloud()->SetData(TargetedPoints);
            TargetedPointCloudComponent->SetHiddenInGame(false);
        }

        if (LabelID == -1)
        {
            CylinderMesh->SetVectorParameterValueOnMaterials("PCTColor", FVector(FLinearColor::White.ToFColorSRGB()));
            TargetedPointCloudComponent->GetPointCloud()->ApplyColorToAllPoints(FLinearColor::White.ToFColorSRGB(), false);
        }
        else
        {
            CylinderMesh->SetVectorParameterValueOnMaterials("PCTColor", FVector(CurrentLabelColor.ToFColorSRGB()));
            TargetedPointCloudComponent->GetPointCloud()->ApplyColorToAllPoints(CurrentLabelColor.ToFColorSRGB(), false);
        }

        FirstTraceBlocked = true;
    }

    else
    {
        FVector NullVector = FVector(0, 0, 0);
        UpdateCylinderTransform(CylinderMesh, AimingLocation, NullVector, AimingDirection, 0.001, false);
        if (LabelID == -1)
        {
            CylinderMesh->SetVectorParameterValueOnMaterials("PCTColor", FVector(FLinearColor::White.ToFColorSRGB()));
        }
        else
        {
            CylinderMesh->SetVectorParameterValueOnMaterials("PCTColor", FVector(CurrentLabelColor.ToFColorSRGB()));
        }
        TargetedPointCloudComponent->GetPointCloud()->Octree.Empty(false);
        TargetedPointCloudComponent->SetHiddenInGame(true);

        FirstTraceBlocked = false;
    }
}

void USphereTraceCascade::SphereTrace(
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
    FVector AimingDirection)
{
    if (DoTrace)
    {
        FHitResult HitResult;

        bool bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(
            OriginalPointCloudComponent->GetWorld(),
            StartLocation,
            EndLocation,
            Radius,
            ObjectTypes,
            true,
            ActorsToIgnore,
            EDrawDebugTrace::None,
            HitResult,
            true,
            FLinearColor::Red,
            FLinearColor::Green,
            5.0f
        );

        if (bHit)
        {
            // Handle hit result
            FVector HitLocation = HitResult.ImpactPoint;

            LocalSphereCenters.Add(HitLocation);
            TArray<FLidarPointCloudPoint*> PointsInSphere;
            OriginalPointCloudComponent->GetPointsInSphere(PointsInSphere, HitLocation, Radius, false);
            LocalTargetedPoints.Append(PointsInSphere);

            if (bLabel)
            {
                OriginalPointCloudComponent->ApplyClassificationToPointsInSphere(uint8(LabelID + 1), HitLocation, Radius, false);
            }
        }
    }

    else
    {
        LocalSphereCenters.Add(FirstImpactPoint);
        TArray<FLidarPointCloudPoint*> PointsInSphere;
        OriginalPointCloudComponent->GetPointsInSphere(PointsInSphere, FirstImpactPoint, Radius, false);
        LocalTargetedPoints.Append(PointsInSphere);

        if (bLabel)
        {
            OriginalPointCloudComponent->ApplyClassificationToPointsInSphere(uint8(LabelID + 1), FirstImpactPoint, Radius, false);
        }
    }
}

void USphereTraceCascade::UpdateCylinderTransform(UStaticMeshComponent* CylinderMesh, FVector& Start, FVector& End, FVector& Direction, float Radius, bool bHit)
{
    if (CylinderMesh)
    {
        if (bHit)
        {
            float Length = (End - Start).Size();

            // Calculate rotation to align cylinder with direction
            FRotator Rotation = Direction.Rotation().Add(90, 0, 0);

            // Create transform
            FTransform NewTransform;
            NewTransform.SetLocation(Start + Direction * Length / 2.0f); // Midpoint
            NewTransform.SetRotation(Rotation.Quaternion());
            NewTransform.SetScale3D(FVector(Radius, Radius, Length / 100.0f));

            // Update cylinder transform
            CylinderMesh->SetWorldTransform(NewTransform);
        }

        else
        {
            // Calculate rotation to align cylinder with direction
            FRotator Rotation = Direction.Rotation().Add(90, 0, 0);

            // Create transform
            FTransform NewTransform;
            NewTransform.SetLocation(Start + Direction * 500); // Midpoint
            NewTransform.SetRotation(Rotation.Quaternion());
            NewTransform.SetScale3D(FVector(Radius, Radius, 10));

            // Update cylinder transform
            CylinderMesh->SetWorldTransform(NewTransform);
        }
    }
}