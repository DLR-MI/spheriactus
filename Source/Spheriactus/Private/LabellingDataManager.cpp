// Copyright (c) Bruno Pereira Costa

#include "LabellingDataManager.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Kismet/KismetArrayLibrary.h"
#include "UObject/UnrealType.h"
#include "Algo/Count.h"

void ULabellingDataManager::ExportLabelledPointCloud(ULidarPointCloud* LabelledPointCloud, const TArray<FString>& LabelNames, const FString& FilePath)
{
    TArray<FLidarPointCloudPoint> LabelledPoints;
    LabelledPointCloud->RestoreOriginalCoordinates();
    LabelledPointCloud->GetPointsAsCopies(LabelledPoints, true);

    // Verify we got points
    if (LabelledPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No points retrieved from point cloud"));
        return;
    }

    FString ExportDataContent;

    // Add header
    ExportDataContent += TEXT("x\ty\tz\tr\tg\tb\tLabel_ID\tLabel_Name\tid\n");

    // Get the current instance of ULidarPointCloudSettings to access ExportScale variable
    ULidarPointCloudSettings* Settings = GetMutableDefault<ULidarPointCloudSettings>();
    float ExportScale = Settings ? Settings->ExportScale : 0.01f;

    // Map to store the label names
    TMap<int32, FString> LabelNameMap;
    LabelNameMap.Add(0, "Unlabeled");
    for (int i = 0; i < LabelNames.Num(); i++)
    {
        LabelNameMap.Add(i + 1, LabelNames[i]);
    }

    for (const auto& Point : LabelledPoints)
    {
        ExportDataContent += FString::Printf(TEXT("%.8f\t%.8f\t%.8f\t%d\t%d\t%d\t%d\t\"%s\"\t%d\n"),
            Point.PointID,
            Point.Location.X * ExportScale, -Point.Location.Y * ExportScale, Point.Location.Z * ExportScale,
            Point.Color.R, Point.Color.G, Point.Color.B,
            Point.ClassificationID,
            *LabelNameMap[Point.ClassificationID]);
    }

    FFileHelper::SaveStringToFile(ExportDataContent, *FilePath);
}

void ULabellingDataManager::ExportLabelledPointCloudComponent(const ULidarPointCloudComponent* LabelledPointCloudComponent, const TArray<FString>& LabelNames, const FString& FilePath, const FString& OriginalFilePath)
{
    if (!LabelledPointCloudComponent || !LabelledPointCloudComponent->GetPointCloud())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid point cloud component"));
        return;
    }

    TArray<FLidarPointCloudPoint*> LabelledPointsPtr;
    LabelledPointCloudComponent->GetPointCloud()->GetPoints(LabelledPointsPtr);

    // Verify we got points
    if (LabelledPointsPtr.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No points retrieved from point cloud"));
        return;
    }

    // Read original file into an array of lines
    TArray<FString> FileLines;
    if (!FFileHelper::LoadFileToStringArray(FileLines, *OriginalFilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to read original file: %s"), *OriginalFilePath);
        return;
    }

    // Detect header presence
    int32 HeaderLines = 0;
    TArray<FString> FirstLineTokens;
    FileLines[0].ParseIntoArray(FirstLineTokens, TEXT(" 	,"), true); // space, tab or comma

    TArray<FString> ExpectedHeaders = { "x", "y", "z", "r", "g", "b" };
    for (const FString& Token : FirstLineTokens)
    {
        for (const FString& Header : ExpectedHeaders)
        {
            if (Token.Equals(Header, ESearchCase::IgnoreCase))
            {
                HeaderLines = 1;
                break;
            }
        }
        if (HeaderLines > 0) break;
    }

    // Determine delimiter dynamically from the first data line
    // Determine delimiter by counting occurrences in the first data line
    FString FirstDataLine = FileLines[HeaderLines];
    int32 Commas = Algo::Count(FirstDataLine, TCHAR(','));
    int32 Tabs = Algo::Count(FirstDataLine, TCHAR('\t'));
    int32 Spaces = Algo::Count(FirstDataLine, TCHAR(' '));
    TCHAR Delimiter = '\t';
    if (Commas >= Tabs && Commas >= Spaces)        Delimiter = ',';
    else if (Tabs > Commas && Tabs >= Spaces)     Delimiter = '\t';
    else                                          Delimiter = ' ';
    //UE_LOG(LogTemp, Warning, TEXT("Detected delimiter: '%c' (code %d)"), Delimiter, (int32)Delimiter);
    // Map to store the label names with check for sufficient labels
    int32 NumberUniqueClassificationsImported = LabelledPointCloudComponent->GetPointCloud()->GetClassificationsImported().Num();
    TMap<int32, FString> LabelNameMap;
    LabelNameMap.Add(0, "unlabeled");
    if (NumberUniqueClassificationsImported <= LabelNames.Num())
    {
        for (int i = 0; i < LabelNames.Num(); i++)
        {
            LabelNameMap.Add(i + 1, LabelNames[i]);
        }
    }
    else
    {
        for (int i = 0; i < NumberUniqueClassificationsImported; i++)
        {
            LabelNameMap.Add(i + 1, FString::Printf(TEXT("label_%d"), i));
        }
    }


    // Write to file in batches
    const int32 BatchSize = 1000000; // Export in batches of 1 million points
    FString ExportDataContent;

    // Add header
    ExportDataContent += TEXT("x\ty\tz\tr\tg\tb\tlabel_id\tlabel_name\n");

    for (int32 i = 0; i < LabelledPointsPtr.Num(); i += BatchSize)
    {
        for (int32 j = i; j < FMath::Min(i + BatchSize, LabelledPointsPtr.Num()); j++)
        {
            const FLidarPointCloudPoint* Point = LabelledPointsPtr[j];
            if (!Point) continue;

            int32 LineIndex = Point->PointID + HeaderLines; // Adjust for header
            if (LineIndex >= FileLines.Num()) continue;

            TArray<FString> OriginalTokens;
            double X, Y, Z;

            if (FileLines.IsValidIndex(LineIndex))
            {
                const TCHAR Delims[2] = { Delimiter, TEXT('\0') };
                FileLines[LineIndex].ParseIntoArray(OriginalTokens, Delims, true);

                X = FCString::Atod(*OriginalTokens[0]);
                Y = FCString::Atod(*OriginalTokens[1]);
                Z = FCString::Atod(*OriginalTokens[2]);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("PointID %d is out of bounds in original file."), Point->PointID);
                continue;
            }

            ExportDataContent += FString::Printf(TEXT("%.8f\t%.8f\t%.8f\t%d\t%d\t%d\t%d\t\"%s\"\n"),
                X, Y, Z,
                Point->Color.R, Point->Color.G, Point->Color.B,
                Point->ClassificationID, *LabelNameMap[Point->ClassificationID]);
        }

        if (ExportDataContent.Len() > 0)
        {
            if (i == 0)
            {
                // Save the batch to the file by re-writing its content
                FFileHelper::SaveStringToFile(ExportDataContent, *FilePath);
            }
            else
            {
                // Append the data to the file
                FFileHelper::SaveStringToFile(ExportDataContent, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
            }
        }

        ExportDataContent.Reset();
    }
}