// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EllipsoidWoundComponent.generated.h"

struct FWound
{
	FFloat16Color Position;
	FFloat16Color ForwardVector;
	FFloat16Color SideVector;
	FFloat16Color UpVector;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEllipsoidWoundComponentRTCreated, UTextureRenderTarget2D*, WoundRT);

UCLASS( ClassGroup=(Rendering), meta=(BlueprintSpawnableComponent) )
class ELLIPSOIDWOUNDS_API UEllipsoidWoundComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEllipsoidWoundComponent();

	virtual void BeginPlay() override;

	void CreateRT();

	UFUNCTION(BlueprintCallable, Category = "Ellipsoid Wound Component")
		void AddWound(const FHitResult& WoundHitResult, float WoundDepth = 8.f, float WoundWidth = 1.f, float WoundHeight = 1.f);

	UPROPERTY(BlueprintAssignable, Category = "Ellipsoid Wound Component")
		FOnEllipsoidWoundComponentRTCreated OnRTCreated;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Transient, Category = "Ellipsoid Wound Component")
		class UTextureRenderTarget2D* WoundRT;

	// This changes the size of the WoundRT. The size of the WoundRT = (x = MaximumNumberOfWounds * 4, y = 1)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ellipsoid Wound Component")
		int32 MaximumNumberOfWounds;

	// The WoundRT is treated as a circular buffer
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Ellipsoid Wound Component")
		int32 CurrentWoundIndex;

	// Which skeletal mesh component to apply the wounds to. Put in the name of the component here
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ellipsoid Wound Component", meta = (AllowedClasses = "/Script/Engine.SkeletalMeshComponent"))
		FComponentReference WoundedComponent;
};
