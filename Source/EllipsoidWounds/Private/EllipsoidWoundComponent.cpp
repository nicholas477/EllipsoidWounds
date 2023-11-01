// Fill out your copyright notice in the Description page of Project Settings.


#include "EllipsoidWoundComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetSystemLibrary.h"
#include "EllipsoidWoundsFunctionLibrary.h"

UEllipsoidWoundComponent::UEllipsoidWoundComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MaximumNumberOfWounds = 16;
	CurrentWoundIndex = 0;

    WoundedComponent.ComponentProperty = "Mesh";
}

void UEllipsoidWoundComponent::BeginPlay()
{
	Super::BeginPlay();
	CreateRT();
}

void UEllipsoidWoundComponent::CreateRT()
{
    if (!UKismetSystemLibrary::IsDedicatedServer(this) && MaximumNumberOfWounds > 0 && GetWorld())
    {
        WoundRT = NewObject<UTextureRenderTarget2D>(GetWorld(), UTextureRenderTarget2D::StaticClass());
        check(WoundRT);
        WoundRT->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA32f;
        WoundRT->ClearColor = FLinearColor::Transparent;
        WoundRT->bAutoGenerateMips = false;
        WoundRT->InitCustomFormat(MaximumNumberOfWounds * 4, 1, EPixelFormat::PF_FloatRGBA, true);
        WoundRT->SRGB = false;
        WoundRT->Filter = TextureFilter::TF_Nearest;
        WoundRT->UpdateResourceImmediate(true);

        OnRTCreated.Broadcast(WoundRT);
    }
}

static void UpdateTextureRegion(FTexture2DRHIRef TextureRHI, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D Region, FWound Wound)
{
    ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)(
        [=](FRHICommandListImmediate& RHICmdList)
        {
            check(TextureRHI.IsValid());

            uint32 SrcPitch, SrcBpp;
            // 8 bytes per channel, 4 channels per pixel
            SrcBpp = 8 * 4;
            SrcPitch = TextureRHI->GetSizeX() * SrcBpp;

            RHIUpdateTexture2D(
                TextureRHI,
                MipIndex,
                Region,
                SrcPitch,
                (uint8*)&Wound
                + Region.SrcY * SrcPitch
                + Region.SrcX * SrcBpp
            );
        });
}

static FFloat16Color FVector4ToHalfColor(const FVector4& Vector)
{
    FFloat16Color OutColor;
    OutColor.R = Vector.X;
    OutColor.G = Vector.Y;
    OutColor.B = Vector.Z;
    OutColor.A = Vector.W;
    return OutColor;
}

void UEllipsoidWoundComponent::AddWound(const FHitResult& WoundHitResult, float WoundDepth, float WoundWidth, float WoundHeight)
{
    if (WoundRT)
    {
        const USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(WoundedComponent.GetComponent(GetOwner()));
        if (WoundHitResult.GetComponent() == SkeletalMeshComponent)
        {
            const FVector ShotDirection = (WoundHitResult.TraceEnd - WoundHitResult.TraceStart).GetSafeNormal();

            const FVector PoseSpacePosition = UEllipsoidWoundsFunctionLibrary::TransformPositionToRefPose(SkeletalMeshComponent, WoundHitResult.BoneName, WoundHitResult.Location);
            const FVector PoseSpaceShotDirection = UEllipsoidWoundsFunctionLibrary::TransformDirectionToRefPose(SkeletalMeshComponent, WoundHitResult.BoneName, ShotDirection);
            const FVector PoseSpaceShotDirectionSideVector = FVector(PoseSpaceShotDirection.Y * -1.f, PoseSpaceShotDirection.X, PoseSpaceShotDirection.Z);
            const FVector PoseSpaceShotDirectionUpVector = FVector::CrossProduct(PoseSpaceShotDirection, PoseSpaceShotDirectionSideVector);

            FWound NewWound;
            NewWound.Position = FVector4ToHalfColor(FVector4(PoseSpacePosition, 1.f));
            NewWound.ForwardVector = FVector4ToHalfColor(FVector4(PoseSpaceShotDirection * (1.f / WoundDepth), 1.f));
            NewWound.SideVector = FVector4ToHalfColor(FVector4(PoseSpaceShotDirectionSideVector * (1.f / WoundWidth), 1.f));
            NewWound.UpVector = FVector4ToHalfColor(FVector4(PoseSpaceShotDirectionUpVector * (1.f / WoundHeight), 1.f));

            uint32 WrappedCurrentWoundIndex = CurrentWoundIndex % MaximumNumberOfWounds;

            auto Region = FUpdateTextureRegion2D(4 * WrappedCurrentWoundIndex, 0, 0, 0, 4, 1);

            FTextureRenderTarget2DResource* TextureResource = (FTextureRenderTarget2DResource*)WoundRT->GetResource();
            check(TextureResource);
            check(TextureResource->GetRenderTargetTexture());
            UpdateTextureRegion(TextureResource->GetRenderTargetTexture(), 0, 1, Region, NewWound);

            CurrentWoundIndex++;
        }
    }
}
