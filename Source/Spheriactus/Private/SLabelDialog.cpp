// Copyright (c) Bruno Pereira Costa

#include "SLabelDialog.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Application/SlateApplication.h"

#include "SStartWidgetSwitcher.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SLabelDialog::Construct(const FArguments& InArgs)
{
    StartWidgetSwitcher = InArgs._StartWidgetSwitcher;

    GridLineBrush = MakeShared<FSlateColorBrush>(FLinearColor(0.203300, 0.203300, 0.203300));

    const float CornerRadius = 4.0f;
    SolidPickerBrush = MakeShared<FSlateRoundedBoxBrush>(
        FLinearColor::White,     // Fill color
        CornerRadius             // Corner radius
    );


    // Add initial entries to the map
    LabelEntries.Add("Label", FLinearColor::Red);

    // Populate the EntryKeys array with keys from the map
    for (const auto& Entry : LabelEntries)
    {
        EntryKeys.Add(MakeShareable(new FString(Entry.Key)));
    }

    // Set up the widget layout
    ChildSlot
        [
            SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SBorder)
                        .BorderImage(FAppStyle::Get().GetBrush("DetailsView.GridLine"))
                        .Clipping(EWidgetClipping::ClipToBounds)
                        .BorderBackgroundColor(FLinearColor(1, 0.5, 0, 1))
                        [
                            SNew(SBox)
                                .MinDesiredHeight(26.0F)
                                [
                                    SNew(SHorizontalBox)
                                        + SHorizontalBox::Slot()
                                        .VAlign(VAlign_Center)
                                        .Padding(4.0f, 0.0f, 0.0f, 0.0f)
                                        .AutoWidth()
                                        [
                                            SNew(STextBlock)
                                                .Text(FText::FromString("Please press the 'Add Entry' button to add labels and edit them"))
                                                .Font(FAppStyle::GetFontStyle(TEXT("PropertyWindow.NormalFont")))
                                        ]
                                ]
                        ]
                ]

                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                [
                    SAssignNew(ListView, SListView<TSharedPtr<FString>>)
                        .ListItemsSource(&EntryKeys)
                        .OnGenerateRow(this, &SLabelDialog::OnGenerateRow)
                ]

                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .HAlign(HAlign_Left)
                        .Padding(5.0f)
                        [
                            SNew(SButton)
                                .Text(FText::FromString("Add Entry"))
                                .OnClicked_Lambda([this]() -> FReply {
                                AddEntry();
                                return FReply::Handled();
                                    })
                        ]
                        + SHorizontalBox::Slot()
                        .HAlign(HAlign_Right)
                        .Padding(5.0f)
                        [
                            SNew(SButton)
                                .Text(FText::FromString("Next"))
                                .OnClicked_Lambda([this]() -> FReply {
                                OnNextButtonClicked();
                                return FReply::Handled();
                                    })
                        ]
                ]
        ];
}

TSharedRef<ITableRow> SLabelDialog::OnGenerateRow(TSharedPtr<FString> InKey, const TSharedRef<STableViewBase>& OwnerTable)
{
    return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
        [
            SNew(SBorder)
                .BorderImage(FAppStyle::Get().GetBrush("DetailsView.GridLine"))
                .Padding(FMargin(0, 3, 0, 0))
                .Clipping(EWidgetClipping::ClipToBounds)
                [
                    SNew(SBox)
                        .MinDesiredHeight(26.0F)
                        [
                            SNew(SHorizontalBox)
                                + SHorizontalBox::Slot()
                                .FillWidth(1.0f)
                                .Padding(FMargin(2.0f, 2.0f, 4.0f, 2.0f))
                                [
                                    SNew(SEditableTextBox)
                                        .Text(FText::FromString(*InKey))
                                        .OnTextCommitted_Lambda([this, InKey](const FText& NewText, ETextCommit::Type CommitType) {
                                        FString NewKey = NewText.ToString();
                                        if (NewKey != *InKey)
                                        {
                                            FLinearColor Color = LabelEntries[*InKey];
                                            LabelEntries.Remove(*InKey);
                                            LabelEntries.Add(NewKey, Color);
                                            *InKey = NewKey;
                                            ListView->RequestListRefresh();
                                        }
                                            })
                                ]

                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(FMargin(2.0f, 2.0f, 4.0f, 2.0f))
                                [

                                    SNew(SBox)
                                        .Padding(FMargin(0, 0, 4.0f, 0.0f))
                                        .VAlign(VAlign_Center)
                                        [
                                            SNew(SBorder)
                                                .Padding(1)
                                                .BorderImage(SolidPickerBrush.Get())
                                                .BorderBackgroundColor(FLinearColor::Gray)
                                                .VAlign(VAlign_Center)
                                                [
                                                    SNew(SOverlay)
                                                        + SOverlay::Slot()
                                                        .VAlign(VAlign_Center)
                                                        [
                                                            SNew(SColorBlock)
                                                                .AlphaBackgroundBrush(FAppStyle::Get().GetBrush("ColorPicker.RoundedAlphaBackground"))
                                                                .Color_Lambda([this, InKey]() -> FLinearColor {
                                                                return LabelEntries[*InKey];
                                                                    })
                                                                .ShowBackgroundForAlpha(true)
                                                                .AlphaDisplayMode(EColorBlockAlphaDisplayMode::Separate)
                                                                .OnMouseButtonDown_Lambda([this, InKey](const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) -> FReply {
                                                                FColorPickerArgs PickerArgs;
                                                                PickerArgs.bIsModal = true;
                                                                PickerArgs.bUseAlpha = true;
                                                                PickerArgs.InitialColor = LabelEntries[*InKey];
                                                                PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateLambda([this, InKey](FLinearColor NewColor) {
                                                                    LabelEntries[*InKey] = NewColor;
                                                                    });
                                                                OpenColorPicker(PickerArgs);
                                                                return FReply::Handled();
                                                                    })
                                                                .Size(FVector2D(70.0f, 20.0f))
                                                                .CornerRadius(FVector4(4.0f, 4.0f, 4.0f, 4.0f))
                                                        ]
                                                ]
                                        ]
                                ]
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                [
                                    SNew(SBox)
                                        .HAlign(HAlign_Center)
                                        .VAlign(VAlign_Center)
                                        .WidthOverride(22)
                                        .HeightOverride(22)
                                        .Visibility_Lambda([this, InKey]() -> EVisibility {
                                            // If there's only one entry, or if this entry is the first in the list, hide the button (but preserve its space)
                                            if (EntryKeys.Num() <= 1)
                                            {
                                                return EVisibility::Hidden;
                                            }
                                            if (EntryKeys.Num() > 0 && *InKey == *EntryKeys[0])
                                            {
                                                return EVisibility::Hidden;
                                            }
                                            return EVisibility::Visible;
                                        })

                                        
                                        [
                                            SNew(SButton)
                                                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                                                .OnClicked_Lambda([this, InKey]() -> FReply {
                                                RemoveEntry(*InKey);
                                                return FReply::Handled();
                                                    })
                                                .ContentPadding(0)
                                                [
                                                    SNew(SImage)
                                                        .Image(FAppStyle::Get().GetBrush("Symbols.X"))
                                                        .ColorAndOpacity(FSlateColor::UseForeground())
                                                ]
                                        ]
                                ]
                        ]
                ]
        ];
}

void SLabelDialog::AddEntry()
{
    TArray PrefefinedColors = { FLinearColor::Red,
                                FLinearColor::Green,
                                FLinearColor::Blue,
                                FLinearColor(0, 255.0f / 255.0f, 255.0f / 255.0f, 1.0f),
                                FLinearColor(1.0f, 0.0f, 0.75f, 1.0f),
                                FLinearColor(1.0f, 1.0f, 0.0f, 1.0f) };

    FString NewKey = "Label";
    int32 Suffix = 1;
    while (LabelEntries.Contains(NewKey))
    {
        NewKey = FString::Printf(TEXT("Label %d"), Suffix++);
    }
    
    // Collect all currently used colors.
    TSet<FLinearColor> UsedColors;
    for (const auto& Entry : LabelEntries)
    {
        UsedColors.Add(Entry.Value);
    }

    FLinearColor NewColor = FLinearColor(0, 0, 0, 0);
    bool bFoundFreePredefined = false;

    for (const FLinearColor& Color : PrefefinedColors)
    {
        if (!UsedColors.Contains(Color))
        {
            NewColor = Color;
            bFoundFreePredefined = true;
            break;
        }
    }

    // If all predefined colors are taken, generate a random color.
    if (!bFoundFreePredefined)
    {
        float RandomHue = FMath::RandRange(0.0f, 359.0f); // Random hue between 0 and 359
        NewColor = FLinearColor::MakeFromHSV8(RandomHue, 255.0f, 255.0f);
    }
    
    LabelEntries.Add(NewKey, NewColor);
    EntryKeys.Add(MakeShareable(new FString(NewKey)));
    ListView->RequestListRefresh();
}

void SLabelDialog::RemoveEntry(const FString& Key)
{
    LabelEntries.Remove(Key);
    EntryKeys.RemoveAll([&Key](const TSharedPtr<FString>& EntryKey) {
        return *EntryKey == Key;
        });
    ListView->RequestListRefresh();
}

TMap<FString, FLinearColor>* SLabelDialog::GetLabelEntries()
{
    return &LabelEntries;
}

void SLabelDialog::OnNextButtonClicked()
{
    if (StartWidgetSwitcher.IsValid())
    {
        StartWidgetSwitcher.Pin()->CloseWindow();
    }
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION