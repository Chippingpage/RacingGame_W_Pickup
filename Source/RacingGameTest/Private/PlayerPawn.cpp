// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawn.h"
#include "Bullet.h"
#include "CheckPoint.h"
#include "MainSaveGame.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"



// Sets default values
APlayerPawn::APlayerPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	PlayerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerMesh"));
	SetRootComponent(PlayerMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	CollisionBox->SetGenerateOverlapEvents(true);
	CollisionBox->SetupAttachment(PlayerMesh);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 500.f;
	SpringArm->SetRelativeRotation(FRotator(-20.f, 0.f, 0.f));
	SpringArm->bEnableCameraLag = true;
	SpringArm->bEnableCameraRotationLag = true;

	SpringArm->bUsePawnControlRotation = false;

	SpringArm->CameraLagSpeed = 3.f;
	SpringArm->CameraRotationLagSpeed = 3.f;
	SpringArm->CameraLagMaxDistance = 100.f;


	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("BackSpringArm"));
	BackSpringArm->SetupAttachment(RootComponent);
	BackSpringArm->TargetArmLength = 1000.f;
	BackSpringArm->SetRelativeRotation(FRotator(-20.f, 180.f, 0.f));
	BackSpringArm->bEnableCameraLag = true;
	BackSpringArm->bEnableCameraRotationLag = true;

	BackSpringArm->bUsePawnControlRotation = false;

	BackSpringArm->CameraLagSpeed = 3.f;
	BackSpringArm->CameraRotationLagSpeed = 3.f;
	BackSpringArm->CameraLagMaxDistance = 100.f;

	PlayerMesh->SetSimulatePhysics(true);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->AddLocalOffset(FVector(0.0f, 0.0f, 60.0f));

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("BackCamera"));
	BackCamera->SetupAttachment(BackSpringArm, USpringArmComponent::SocketName);
	BackCamera->AddLocalOffset(FVector(0.0f, 0.0f, 60.0f));

	PawnMovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));


	PawnMovementComponent->MaxSpeed = 5000.f;

	AngularDamping = 5.0f;
	LinearDamping = 3.0f;
}

// Called when the game starts or when spawned
void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	CollisionBox = this->FindComponentByClass<UBoxComponent>();

	if (CollisionBox) {
		CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &APlayerPawn::OnOverlap);
	}
}

// Called every frame
void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Forward = GetActorForwardVector();
	Forward.Z = 0;


	if (bDriving)
	{
		AddMovementInput(FVector(Forward), DriveSpeed);
		PlayerMesh->SetAngularDamping(AngularDamping);
		PlayerMesh->SetLinearDamping(LinearDamping);
	}

	if (bBreaking)
	{
		AddMovementInput(FVector(Forward), (-DriveSpeed / 2));
	}

	if (bBoosting)
	{
		BoostAmount -= 0.005f;
		PlayerMesh->AddForce(Forward * BoostPower * PlayerMesh->GetMass());
		if (BoostAmount < 0.f)
		{
			bBoosting = false;
		}
	}



	//UE_LOG(LogTemp, Warning, TEXT("Current BoostFuel: %f"), BoostAmount);
	if (bCanPlay) {
		float MaxDistance = 100.f;
		FVector EndLocation = GetActorLocation() + (GetActorUpVector() * -MaxDistance);
		FHitResult HitResult;
		FCollisionObjectQueryParams CollisionObjectQueryParams;
		CollisionObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_WorldStatic);
		if (GetWorld()->LineTraceSingleByObjectType(HitResult, GetActorLocation(), EndLocation, CollisionObjectQueryParams))
		{
			PlayerMesh->AddForce(GetActorUpVector() * 800.f * PlayerMesh->GetMass());
			//UE_LOG(LogTemp, Warning, TEXT("Tracer works"));
		}
	}

	if (Lap == 4) {

		UGameplayStatics::OpenLevel(GetWorld(), "MainMenuMap");

		UE_LOG(LogTemp, Warning, TEXT("You Win"));
	}
}

// Called to bind functionality to input
void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent->BindAction("Driving", IE_Pressed, this, &APlayerPawn::StartDriving);
	InputComponent->BindAction("Driving", IE_Released, this, &APlayerPawn::StopDriving);

	InputComponent->BindAction("Breaking", IE_Pressed, this, &APlayerPawn::StartBreaking);
	InputComponent->BindAction("Breaking", IE_Released, this, &APlayerPawn::StopBreaking);

	InputComponent->BindAction("Shooting", IE_Pressed, this, &APlayerPawn::Shooting);

	InputComponent->BindAction("Boost", IE_Pressed, this, &APlayerPawn::StartBoosting);
	InputComponent->BindAction("Boost", IE_Released, this, &APlayerPawn::StopBoosting);

	InputComponent->BindAxis("MoveRight", this, &APlayerPawn::MoveRight);

	InputComponent->BindAction("SwitchCamera", IE_Pressed, this, &APlayerPawn::SwitchCamera);

	InputComponent->BindAction("LoadGame", IE_Pressed, this, &APlayerPawn::LoadGame);

	InputComponent->BindAction("Pause", IE_Pressed, this, &APlayerPawn::Pause).bExecuteWhenPaused = true;
}





void APlayerPawn::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA(ACheckPoint::StaticClass()))
	{
		SaveGame();
		CheckPoint++;
		if (CheckPoint == 12) {
			Lap++;
			CheckPoint = 0;
		}
	}

	if (OtherActor->IsA(ABullet::StaticClass()))
	{
		Cast<ABullet>(OtherActor)->Destroy();
		PawnMovementComponent->MaxSpeed -= 500.f;
	}

}

void APlayerPawn::StartDriving()
{
	if (bCanPlay)
	{
		bDriving = true;
	}
}

void APlayerPawn::StopDriving()
{
	bDriving = false;
}

void APlayerPawn::StartBreaking()
{
	if (bCanPlay)
	{
		bBreaking = true;
	}
}

void APlayerPawn::StopBreaking()
{
	bBreaking = false;
}

void APlayerPawn::StartBoosting()
{
	if (bCanPlay) {
		if (BoostAmount > 0)
		{
			bBoosting = true;
		}
	}
}

void APlayerPawn::StopBoosting()
{
	
	bBoosting = false;
}

void APlayerPawn::MoveRight(float Value)
{
	if (bCanPlay)
	{
		FVector Forward = GetActorForwardVector();
		FVector Right = GetActorRightVector();


		CurrentTurnSpeed = FMath::FInterpTo(CurrentTurnSpeed, Value, GetWorld()->GetDeltaSeconds(), 1.f);

		PlayerMesh->AddRelativeRotation(FRotator(0.f, 3.f, 0.f) * CurrentTurnSpeed);
	}
}

void APlayerPawn::Shooting()
{
	if (bCanPlay)
	{
		if (bSwitchCamera) 
		{
			if (Ammo > 0) {
				Ammo--;

				UWorld* World = GetWorld();
				if (World)
				{

					FVector Location = GetActorLocation();
					FRotator Rotation = GetActorRotation();
					FVector FwdVector = GetActorForwardVector();
					FwdVector *= -400;
					Location += FwdVector;
					Rotation.Yaw += 180;
					Rotation.Pitch += 3;
					World->SpawnActor<AActor>(BulletSpawn, Location, Rotation);

				}
				UE_LOG(LogTemp, Warning, TEXT("Shooting"));
			}
		}
	}

}

void APlayerPawn::SwitchCamera()
{
	if (!bSwitchCamera) {

		Camera->Deactivate();
		BackCamera->Activate();

	}
	else
	{
		Camera->Activate();
		BackCamera->Deactivate();
	}
	bSwitchCamera = !bSwitchCamera;
}

void APlayerPawn::SaveGame()
{



	UMainSaveGame* SaveGameInstance = Cast<UMainSaveGame>(UGameplayStatics::CreateSaveGameObject(UMainSaveGame::StaticClass()));

	SaveGameInstance->PlayerPosition = this->GetActorLocation();
	SaveGameInstance->PlayerRotation = this->GetActorRotation();
	SaveGameInstance->BoostAmountSave = this->BoostAmount;

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->PlayerName, SaveGameInstance->PlayerIndex);
	UE_LOG(LogTemp, Warning, TEXT("Saved: %s"), *SaveGameInstance->PlayerName);
}

void APlayerPawn::LoadGame()
{

	UMainSaveGame* LoadGameInstance = Cast<UMainSaveGame>(UGameplayStatics::CreateSaveGameObject(UMainSaveGame::StaticClass()));

	LoadGameInstance = Cast<UMainSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->PlayerName, LoadGameInstance->PlayerIndex));

	this->SetActorLocation(LoadGameInstance->PlayerPosition);
	this->SetActorRotation(LoadGameInstance->PlayerRotation);
	this->BoostAmount = LoadGameInstance->BoostAmountSave;

	UE_LOG(LogTemp, Warning, TEXT("LOADED: %s"), *LoadGameInstance->PlayerName);

	CheckPoint--;
}

void APlayerPawn::Pause()
{

	if (!myGamePaused)
	{
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}
	else
	{
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
	myGamePaused = !myGamePaused;
	UE_LOG(LogTemp, Warning, TEXT("Paused"))
}





