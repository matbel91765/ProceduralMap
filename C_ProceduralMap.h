// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_ProceduralMap.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;

UCLASS()
class PROCEDURALMAP_API AC_ProceduralMap : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AC_ProceduralMap();

	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0))
		int XSize = 0;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0))
		int YSize = 0;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0))
		float ZMultiplier = 1.0f;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0))
		float NoiseScale = 1.0f;

	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0.000001))
		float Scale = 100.0f;
	UPROPERTY(EditAnywhere, Meta = (ClampMin = 0.000001))
		float UVScale = 0.0f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
		UMaterialInterface* Material;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UProceduralMeshComponent* ProceduralMesh;
	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector2D> UV0;

	void CreateVertices();
	void CreateTriangles();
};
