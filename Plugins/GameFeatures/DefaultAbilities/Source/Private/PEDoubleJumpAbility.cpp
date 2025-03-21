// Author: Lucas Vilas-Boas
// Year: 2022
// Repo: https://github.com/lucoiso/UEProject_Elementus

#include "PEDoubleJumpAbility.h"
#include <PEAbilityTags.h>
#include <GameFramework/Character.h>
#include <GameFramework/CharacterMovementComponent.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(PEDoubleJumpAbility)

UPEDoubleJumpAbility::UPEDoubleJumpAbility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::NonInstanced;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("GameplayAbility.Default.DoubleJump"));

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(GlobalTag_RegenBlock_Stamina));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("GameplayAbility.Default.Dash"));
}

void UPEDoubleJumpAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* const OwningCharacter = Cast<ACharacter>(ActorInfo->AvatarActor.Get());

	// Only characters can activate this ability
	if (!IsValid(OwningCharacter))
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	// Check if the player is in air and launch him (second jump) or just do a normal jump (first jump)
	if (!OwningCharacter->GetCharacterMovement()->IsFalling())
	{
		OwningCharacter->Jump();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
	}
	else
	{
		const FVector VFXLocation = OwningCharacter->GetMesh()->GetSocketLocation("Pelvis_Socket");

		FGameplayCueParameters Params;
		Params.Location = VFXLocation;
		ActivateGameplayCues(FGameplayTag::RequestGameplayTag("GameplayCue.Default.DoubleJump"), Params, ActorInfo->AbilitySystemComponent.Get());

		PlayAbilitySoundAtLocation(ActorInfo->AvatarActor.Get(), VFXLocation);

		OwningCharacter->LaunchCharacter(FVector(0.f, 0.f, AbilityMaxRange), false, true);
	}
}

void UPEDoubleJumpAbility::InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);

	// Send the StopJumping event to the player if valid
	if (ACharacter* const OwningCharacter = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
	{
		OwningCharacter->StopJumping();
	}
}
