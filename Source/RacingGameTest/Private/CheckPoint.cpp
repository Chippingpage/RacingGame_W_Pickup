// Fill out your copyright notice in the Description page of Project Settings.


#include "CheckPoint.h"
#include "Components/BoxComponent.h"

// Sets default values
ACheckPoint::ACheckPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	CheckPointMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CheckPointMesh"));
	SetRootComponent(CheckPointMesh);
	
	FVector MeshScaling;
	MeshScaling.X = 24.0f;
	MeshScaling.Y = 5.0f;
	MeshScaling.Z = 0.1f;
		
	RootComponent->SetWorldScale3D(MeshScaling);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollider"));
	CollisionBox->SetGenerateOverlapEvents(true);
	CollisionBox->SetupAttachment(CheckPointMesh);

	FVector ColliScaling;
	ColliScaling.X = 1.5f;
	ColliScaling.Y = 1.5f;
	ColliScaling.Z = 30.0f;

	FVector ColliLocation;
	ColliLocation.X = 0.0f;
	ColliLocation.Y = 0.0f;
	ColliLocation.Z = 800.0f;


	CollisionBox->SetWorldScale3D(ColliScaling);
	CollisionBox->SetWorldLocation(ColliLocation);
}

// Called when the game starts or when spawned
void ACheckPoint::BeginPlay()
{
	Super::BeginPlay();
	

}

// Called every frame
void ACheckPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

