// Copyright (c) Bruno Pereira Costa

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Delegates/Delegate.h"
#include "Kismet/KismetSystemLibrary.h"

class SPHERIACTUS_API SFileDialog
{
public:
    struct OpenFileDialogOutput
    {
        bool bOpened;
        FString OutFileName;

        OpenFileDialogOutput()
            : bOpened(false), OutFileName(TEXT(""))
        {
        }
    };

    static OpenFileDialogOutput OpenFileDialog(const FString& DefaultPath);
};