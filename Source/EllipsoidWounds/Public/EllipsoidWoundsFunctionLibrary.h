// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EllipsoidWoundsFunctionLibrary.generated.h"

class USkeletalMeshComponent;

/**
 * 
 */
UCLASS()
class ELLIPSOIDWOUNDS_API UEllipsoidWoundsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	// Transforms the position of a location on a posed bone to the local location on the reference pose
	UFUNCTION(BlueprintPure, Category = "Ellipsoid Wounds Function Library")
		static FVector TransformPositionToRefPose(const USkeletalMeshComponent* SkeletalMesh, FName BoneName, const FVector& Position);

	// Transforms the direction vector on a posed bone to the local direction vector on the reference pose
	UFUNCTION(BlueprintPure, Category = "Ellipsoid Wounds Function Library")
		static FVector TransformDirectionToRefPose(const USkeletalMeshComponent* SkeletalMesh, FName BoneName, const FVector& Direction, bool bIncludeScale = false);
};
