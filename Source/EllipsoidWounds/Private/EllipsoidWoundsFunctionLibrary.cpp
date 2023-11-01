// Fill out your copyright notice in the Description page of Project Settings.


#include "EllipsoidWoundsFunctionLibrary.h"

#include "Components/SkeletalMeshComponent.h"
#include "Runtime/Launch/Resources/Version.h"

static FTransform GetBoneTransform(const USkeletalMeshComponent* SkelMesh, FName BoneName)
{
    FTransform BoneTransform;

    if (SkelMesh && !BoneName.IsNone())
    {
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
        FReferenceSkeleton& RefSkel = SkelMesh->GetSkinnedAsset()->GetRefSkeleton();
#else
        FReferenceSkeleton& RefSkel = SkelMesh->SkeletalMesh->GetRefSkeleton();
#endif
        BoneTransform = SkelMesh->GetBoneTransform(RefSkel.FindBoneIndex(BoneName));
    }

    return BoneTransform;
}

static FTransform GetWorldSpaceTransform(const FReferenceSkeleton& RefSkel, int32 BoneIdx)
{
    FTransform BoneTransform;

    if (BoneIdx >= 0)
    {
        BoneTransform = RefSkel.GetRefBonePose()[BoneIdx];

        FMeshBoneInfo BoneInfo = RefSkel.GetRefBoneInfo()[BoneIdx];
        if (BoneIdx != 0 && BoneInfo.ParentIndex >= 0)
        {
            BoneTransform *= GetWorldSpaceTransform(RefSkel, BoneInfo.ParentIndex);
        }
    }

    return BoneTransform;
}

static FTransform GetRefPoseBoneTransform(const USkeletalMeshComponent* SkelMesh, FName BoneName)
{
    FTransform BoneTransform;

    if (SkelMesh && !BoneName.IsNone())
    {
        //SkelMesh->ClearRefPoseOverride();
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 3
        FReferenceSkeleton& RefSkel = SkelMesh->GetSkinnedAsset()->GetRefSkeleton();
#else
        FReferenceSkeleton& RefSkel = SkelMesh->SkeletalMesh->GetRefSkeleton();
#endif

        BoneTransform = GetWorldSpaceTransform(RefSkel, RefSkel.FindBoneIndex(BoneName));
    }

    return BoneTransform;
}

FVector UEllipsoidWoundsFunctionLibrary::TransformPositionToRefPose(const USkeletalMeshComponent* SkeletalMesh, FName BoneName, const FVector& Position)
{
    // Transform from world space to bone relative,
    // then transform from bone relative to pose space using the RefPoseBoneTransform
    const FTransform BoneTransform = GetBoneTransform(SkeletalMesh, BoneName);
    const FTransform RefPoseBoneTransform = GetRefPoseBoneTransform(SkeletalMesh, BoneName);

    return RefPoseBoneTransform.TransformPosition(BoneTransform.InverseTransformPosition(Position));
}

FVector UEllipsoidWoundsFunctionLibrary::TransformDirectionToRefPose(const USkeletalMeshComponent* SkeletalMesh, FName BoneName, const FVector& Direction, bool bIncludeScale)
{
    // Transform from world space to bone relative,
    // then transform from bone relative to pose space using the RefPoseBoneTransform
    const FTransform BoneTransform = GetBoneTransform(SkeletalMesh, BoneName);
    const FTransform RefPoseBoneTransform = GetRefPoseBoneTransform(SkeletalMesh, BoneName);

    if (bIncludeScale)
    {
        return RefPoseBoneTransform.InverseTransformVector(BoneTransform.InverseTransformVector(Direction));
    }
    else
    {
        return RefPoseBoneTransform.InverseTransformVectorNoScale(BoneTransform.InverseTransformVectorNoScale(Direction));
    }
}
