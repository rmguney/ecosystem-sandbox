#include "DynamicWeatherSystem.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ADynamicWeatherSystem::ADynamicWeatherSystem()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Create sun light
	SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
	SunLight->SetupAttachment(RootComponent);
	SunLight->SetIntensity(3.0f);
	SunLight->SetLightColor(FLinearColor(1.0f, 0.9f, 0.8f));
	SunLight->SetCastShadows(true);
	SunLight->SetCastVolumetricShadow(true);
	SunLight->SetCastCloudShadows(true);

	// Create sky light
	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(RootComponent);
	SkyLight->SetSourceType(SLS_CapturedScene);
	SkyLight->SetIntensity(1.0f);

	// Create sky dome
	SkyDome = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkyDome"));
	SkyDome->SetupAttachment(RootComponent);
	SkyDome->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkyDome->SetCastShadow(false);

	// Create weather effects
	RainEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RainEffect"));
	RainEffect->SetupAttachment(RootComponent);
	RainEffect->SetAutoActivate(false);

	SnowEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SnowEffect"));
	SnowEffect->SetupAttachment(RootComponent);
	SnowEffect->SetAutoActivate(false);

	FogEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FogEffect"));
	FogEffect->SetupAttachment(RootComponent);
	FogEffect->SetAutoActivate(false);

	// Set default weather
	CurrentWeatherSettings.WeatherType = EWeatherType::Clear;
	TimeOfDaySettings.CurrentHour = 12.0f;
}

void ADynamicWeatherSystem::BeginPlay()
{
	Super::BeginPlay();

	InitializeComponents();
	SetupLumenGlobalIllumination();
	ForceWeatherUpdate();
}

void ADynamicWeatherSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDynamicTimeOfDay)
	{
		UpdateTimeOfDay(DeltaTime);
	}

	if (bDynamicWeather)
	{
		UpdateWeatherEffects(DeltaTime);
	}

	UpdateSunPosition();
	UpdateSkyLighting();
	UpdateWeatherParticles();
	UpdateMaterialParameters();
}

void ADynamicWeatherSystem::InitializeComponents()
{
	// Setup dynamic sky material
	if (SkyDomeMaterial && SkyDome)
	{
		DynamicSkyMaterial = UMaterialInstanceDynamic::Create(SkyDomeMaterial, this);
		SkyDome->SetMaterial(0, DynamicSkyMaterial);
	}

	// Scale sky dome to cover large area
	if (SkyDome)
	{
		SkyDome->SetWorldScale3D(FVector(100.0f));
	}

	// Find or create directional light and sky light actors
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		SunLightActor = Cast<ADirectionalLight>(FoundActors[0]);
	}

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundActors);
	if (FoundActors.Num() > 0)
	{
		SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
	}
}

void ADynamicWeatherSystem::SetupLumenGlobalIllumination()
{
	if (!bEnableLumenGI)
	{
		return;
	}

	// Enable Lumen through console commands
	if (GEngine)
	{
		// Enable Lumen Global Illumination
		GEngine->Exec(GetWorld(), TEXT("r.DynamicGlobalIlluminationMethod 1"));
		
		// Enable Lumen Reflections
		GEngine->Exec(GetWorld(), TEXT("r.ReflectionMethod 1"));
		
		// Set Lumen quality settings
		FString LumenScaleCommand = FString::Printf(TEXT("r.LumenScene.GlobalLightingAtlas.EmissiveTexelsPerUnrealUnit %f"), LumenSceneDetailScale);
		GEngine->Exec(GetWorld(), *LumenScaleCommand);
		
		FString FinalGatherCommand = FString::Printf(TEXT("r.Lumen.DiffuseIndirect.Allow %d"), bEnableLumenGI ? 1 : 0);
		GEngine->Exec(GetWorld(), *FinalGatherCommand);
		
		// Enable software ray tracing for better quality
		GEngine->Exec(GetWorld(), TEXT("r.Lumen.TraceMeshSDFs 1"));
		
		// Enable Lumen for sky lighting
		GEngine->Exec(GetWorld(), TEXT("r.SkyLight.RealTimeReflectionCapture 1"));
		
		UE_LOG(LogTemp, Warning, TEXT("Lumen Global Illumination enabled"));
	}

	// Configure sky light for Lumen
	if (SkyLight)
	{
		SkyLight->SetRealTimeCaptureEnabled(true);
		SkyLight->SetSourceType(SLS_CapturedScene);
		SkyLight->RecaptureSky();
	}
}

void ADynamicWeatherSystem::SetWeather(EWeatherType NewWeather, float TransitionTime)
{
	if (NewWeather == CurrentWeatherSettings.WeatherType)
	{
		return;
	}

	EWeatherType PreviousWeather = CurrentWeatherSettings.WeatherType;
	
	// Setup transition
	TargetWeatherSettings = CurrentWeatherSettings;
	TargetWeatherSettings.WeatherType = NewWeather;
	
	// Configure target weather settings
	switch (NewWeather)
	{
		case EWeatherType::Clear:
			TargetWeatherSettings.CloudCoverage = 0.1f;
			TargetWeatherSettings.RainIntensity = 0.0f;
			TargetWeatherSettings.FogDensity = 0.0f;
			TargetWeatherSettings.LightIntensity = 3.0f;
			TargetWeatherSettings.SkyTint = FLinearColor(0.5f, 0.7f, 1.0f);
			TargetWeatherSettings.Temperature = 0.7f;
			break;
			
		case EWeatherType::Cloudy:
			TargetWeatherSettings.CloudCoverage = 0.6f;
			TargetWeatherSettings.RainIntensity = 0.0f;
			TargetWeatherSettings.FogDensity = 0.1f;
			TargetWeatherSettings.LightIntensity = 2.0f;
			TargetWeatherSettings.SkyTint = FLinearColor(0.6f, 0.7f, 0.8f);
			TargetWeatherSettings.Temperature = 0.5f;
			break;
			
		case EWeatherType::LightRain:
			TargetWeatherSettings.CloudCoverage = 0.8f;
			TargetWeatherSettings.RainIntensity = 0.3f;
			TargetWeatherSettings.FogDensity = 0.2f;
			TargetWeatherSettings.LightIntensity = 1.5f;
			TargetWeatherSettings.SkyTint = FLinearColor(0.4f, 0.5f, 0.6f);
			TargetWeatherSettings.Temperature = 0.4f;
			break;
			
		case EWeatherType::HeavyRain:
			TargetWeatherSettings.CloudCoverage = 1.0f;
			TargetWeatherSettings.RainIntensity = 0.8f;
			TargetWeatherSettings.FogDensity = 0.4f;
			TargetWeatherSettings.LightIntensity = 0.8f;
			TargetWeatherSettings.SkyTint = FLinearColor(0.3f, 0.4f, 0.5f);
			TargetWeatherSettings.Temperature = 0.3f;
			break;
			
		case EWeatherType::Storm:
			TargetWeatherSettings.CloudCoverage = 1.0f;
			TargetWeatherSettings.RainIntensity = 1.0f;
			TargetWeatherSettings.FogDensity = 0.3f;
			TargetWeatherSettings.LightIntensity = 0.5f;
			TargetWeatherSettings.SkyTint = FLinearColor(0.2f, 0.3f, 0.4f);
			TargetWeatherSettings.WindStrength = 0.8f;
			TargetWeatherSettings.Temperature = 0.2f;
			break;
			
		case EWeatherType::Fog:
			TargetWeatherSettings.CloudCoverage = 0.7f;
			TargetWeatherSettings.RainIntensity = 0.0f;
			TargetWeatherSettings.FogDensity = 0.8f;
			TargetWeatherSettings.LightIntensity = 1.0f;
			TargetWeatherSettings.SkyTint = FLinearColor(0.7f, 0.7f, 0.7f);
			TargetWeatherSettings.Temperature = 0.4f;
			break;
			
		case EWeatherType::Snow:
			TargetWeatherSettings.CloudCoverage = 0.9f;
			TargetWeatherSettings.RainIntensity = 0.0f;
			TargetWeatherSettings.FogDensity = 0.2f;
			TargetWeatherSettings.LightIntensity = 2.0f;
			TargetWeatherSettings.SkyTint = FLinearColor(0.8f, 0.9f, 1.0f);
			TargetWeatherSettings.Temperature = 0.1f;
			break;
	}

	WeatherTransitionDuration = TransitionTime;
	WeatherTransitionTimer = 0.0f;
	bWeatherTransitioning = true;

	OnWeatherChanged.Broadcast(NewWeather, PreviousWeather);
}

void ADynamicWeatherSystem::SetTimeOfDay(float Hour)
{
	TimeOfDaySettings.CurrentHour = FMath::Fmod(Hour, 24.0f);
	UpdateSunPosition();
}

ETimeOfDay ADynamicWeatherSystem::GetTimeOfDayEnum() const
{
	float Hour = TimeOfDaySettings.CurrentHour;
	
	if (Hour >= 5.0f && Hour < 7.0f)
		return ETimeOfDay::Dawn;
	else if (Hour >= 7.0f && Hour < 11.0f)
		return ETimeOfDay::Morning;
	else if (Hour >= 11.0f && Hour < 14.0f)
		return ETimeOfDay::Noon;
	else if (Hour >= 14.0f && Hour < 18.0f)
		return ETimeOfDay::Afternoon;
	else if (Hour >= 18.0f && Hour < 20.0f)
		return ETimeOfDay::Dusk;
	else
		return ETimeOfDay::Night;
}

void ADynamicWeatherSystem::ForceWeatherUpdate()
{
	UpdateSunPosition();
	UpdateSkyLighting();
	UpdateWeatherParticles();
	UpdateMaterialParameters();
	UpdateLumenSettings();
}

void ADynamicWeatherSystem::UpdateLumenSettings()
{
	if (!bEnableLumenGI || !GEngine)
	{
		return;
	}

	// Update Lumen settings based on weather conditions
	float WeatherLightScale = CurrentWeatherSettings.LightIntensity / 3.0f;
	
	FString SceneDetailCommand = FString::Printf(TEXT("r.Lumen.DiffuseIndirect.Allow %d"), bEnableLumenGI ? 1 : 0);
	GEngine->Exec(GetWorld(), *SceneDetailCommand);
	
	// Adjust Lumen quality based on weather
	float QualityScale = FMath::Lerp(0.5f, 1.5f, WeatherLightScale);
	FString QualityCommand = FString::Printf(TEXT("r.Lumen.Reflections.Allow %d"), bEnableLumenGI ? 1 : 0);
	GEngine->Exec(GetWorld(), *QualityCommand);

	// Recapture sky light for Lumen updates
	if (SkyLight)
	{
		SkyLight->RecaptureSky();
	}
}

float ADynamicWeatherSystem::GetTemperatureEffect() const
{
	return CurrentWeatherSettings.Temperature;
}

float ADynamicWeatherSystem::GetVisibilityModifier() const
{
	float Visibility = 1.0f;
	Visibility -= CurrentWeatherSettings.FogDensity * 0.7f;
	Visibility -= CurrentWeatherSettings.RainIntensity * 0.3f;
	Visibility -= CurrentWeatherSettings.CloudCoverage * 0.2f;
	
	// Night time visibility reduction
	ETimeOfDay CurrentTime = GetTimeOfDayEnum();
	if (CurrentTime == ETimeOfDay::Night || CurrentTime == ETimeOfDay::Dawn)
	{
		Visibility *= 0.3f;
	}
	
	return FMath::Clamp(Visibility, 0.1f, 1.0f);
}

float ADynamicWeatherSystem::GetMovementModifier() const
{
	float Movement = 1.0f;
	Movement -= CurrentWeatherSettings.RainIntensity * 0.2f;
	Movement -= CurrentWeatherSettings.WindStrength * 0.1f;
	Movement -= CurrentWeatherSettings.FogDensity * 0.1f;
	
	// Temperature effects
	if (CurrentWeatherSettings.Temperature < 0.3f) // Cold
	{
		Movement *= 0.9f;
	}
	else if (CurrentWeatherSettings.Temperature > 0.8f) // Hot
	{
		Movement *= 0.95f;
	}
	
	return FMath::Clamp(Movement, 0.5f, 1.0f);
}

void ADynamicWeatherSystem::UpdateTimeOfDay(float DeltaTime)
{
	float HourIncrement = (24.0f / DayDuration) * DeltaTime;
	TimeOfDaySettings.CurrentHour += HourIncrement;
	
	if (TimeOfDaySettings.CurrentHour >= 24.0f)
	{
		TimeOfDaySettings.CurrentHour -= 24.0f;
	}

	ETimeOfDay NewTimeOfDay = GetTimeOfDayEnum();
	if (NewTimeOfDay != CurrentTimeOfDay)
	{
		PreviousTimeOfDay = CurrentTimeOfDay;
		CurrentTimeOfDay = NewTimeOfDay;
		OnTimeOfDayChanged.Broadcast(NewTimeOfDay, TimeOfDaySettings.CurrentHour);
	}
}

void ADynamicWeatherSystem::UpdateWeatherEffects(float DeltaTime)
{
	// Handle weather transitions
	if (bWeatherTransitioning)
	{
		WeatherTransitionTimer += DeltaTime;
		float Alpha = WeatherTransitionTimer / WeatherTransitionDuration;
		
		if (Alpha >= 1.0f)
		{
			Alpha = 1.0f;
			bWeatherTransitioning = false;
			CurrentWeatherSettings = TargetWeatherSettings;
		}
		else
		{
			// Interpolate weather settings
			CurrentWeatherSettings.CloudCoverage = FMath::Lerp(CurrentWeatherSettings.CloudCoverage, TargetWeatherSettings.CloudCoverage, Alpha);
			CurrentWeatherSettings.RainIntensity = FMath::Lerp(CurrentWeatherSettings.RainIntensity, TargetWeatherSettings.RainIntensity, Alpha);
			CurrentWeatherSettings.FogDensity = FMath::Lerp(CurrentWeatherSettings.FogDensity, TargetWeatherSettings.FogDensity, Alpha);
			CurrentWeatherSettings.LightIntensity = FMath::Lerp(CurrentWeatherSettings.LightIntensity, TargetWeatherSettings.LightIntensity, Alpha);
			CurrentWeatherSettings.SkyTint = FMath::Lerp(CurrentWeatherSettings.SkyTint, TargetWeatherSettings.SkyTint, Alpha);
			CurrentWeatherSettings.Temperature = FMath::Lerp(CurrentWeatherSettings.Temperature, TargetWeatherSettings.Temperature, Alpha);
		}
	}

	// Random weather changes
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastWeatherChangeTime > WeatherChangeInterval)
	{
		EWeatherType NewWeather = GenerateRandomWeather();
		SetWeather(NewWeather, 10.0f);
		LastWeatherChangeTime = CurrentTime;
	}
}

void ADynamicWeatherSystem::UpdateSunPosition()
{
	if (!SunLight)
		return;

	// Calculate sun angle based on time of day
	float SunAngle = (TimeOfDaySettings.CurrentHour - 6.0f) * 15.0f; // 15 degrees per hour, sunrise at 6 AM
	SunAngle = FMath::Clamp(SunAngle, -90.0f, 90.0f);

	// Set sun rotation
	FRotator SunRotation = FRotator(SunAngle, 180.0f, 0.0f);
	SunLight->SetWorldRotation(SunRotation);

	// Update light intensity based on sun angle
	float IntensityMultiplier = FMath::Cos(FMath::DegreesToRadians(FMath::Abs(SunAngle)));
	IntensityMultiplier = FMath::Max(0.1f, IntensityMultiplier);

	float FinalIntensity = TimeOfDaySettings.SunIntensity * IntensityMultiplier * CurrentWeatherSettings.LightIntensity;
	SunLight->SetIntensity(FinalIntensity);

	// Update light color based on time of day
	FLinearColor LightColor = TimeOfDaySettings.SunColor;
	if (GetTimeOfDayEnum() == ETimeOfDay::Dawn || GetTimeOfDayEnum() == ETimeOfDay::Dusk)
	{
		LightColor = FLinearColor(1.0f, 0.6f, 0.4f); // Warmer colors during dawn/dusk
	}
	else if (GetTimeOfDayEnum() == ETimeOfDay::Night)
	{
		LightColor = TimeOfDaySettings.MoonColor;
		FinalIntensity = TimeOfDaySettings.MoonIntensity;
	}

	SunLight->SetLightColor(LightColor * CurrentWeatherSettings.SkyTint);
}

void ADynamicWeatherSystem::UpdateSkyLighting()
{
	if (!SkyLight)
		return;

	// Update sky light intensity
	float SkyIntensity = CurrentWeatherSettings.LightIntensity * 0.5f;
	SkyLight->SetIntensity(SkyIntensity);

	// Update sky light color
	SkyLight->SetLightColor(CurrentWeatherSettings.SkyTint);

	// Recapture sky for dynamic changes
	SkyLight->RecaptureSky();
}

void ADynamicWeatherSystem::UpdateWeatherParticles()
{
	// Rain effects
	if (RainEffect)
	{
		if (CurrentWeatherSettings.RainIntensity > 0.1f)
		{
			if (!RainEffect->IsActive())
			{
				RainEffect->Activate();
			}
			RainEffect->SetFloatParameter(TEXT("Intensity"), CurrentWeatherSettings.RainIntensity);
		}
		else if (RainEffect->IsActive())
		{
			RainEffect->Deactivate();
		}
	}

	// Snow effects
	if (SnowEffect)
	{
		if (CurrentWeatherSettings.WeatherType == EWeatherType::Snow)
		{
			if (!SnowEffect->IsActive())
			{
				SnowEffect->Activate();
			}
			SnowEffect->SetFloatParameter(TEXT("Intensity"), 1.0f);
		}
		else if (SnowEffect->IsActive())
		{
			SnowEffect->Deactivate();
		}
	}

	// Fog effects
	if (FogEffect)
	{
		if (CurrentWeatherSettings.FogDensity > 0.1f)
		{
			if (!FogEffect->IsActive())
			{
				FogEffect->Activate();
			}
			FogEffect->SetFloatParameter(TEXT("Density"), CurrentWeatherSettings.FogDensity);
		}
		else if (FogEffect->IsActive())
		{
			FogEffect->Deactivate();
		}
	}
}

void ADynamicWeatherSystem::UpdateMaterialParameters()
{
	if (!DynamicSkyMaterial)
		return;

	// Update sky dome material parameters
	DynamicSkyMaterial->SetVectorParameterValue(TEXT("SkyTint"), CurrentWeatherSettings.SkyTint);
	DynamicSkyMaterial->SetScalarParameterValue(TEXT("CloudCoverage"), CurrentWeatherSettings.CloudCoverage);
	DynamicSkyMaterial->SetScalarParameterValue(TEXT("FogDensity"), CurrentWeatherSettings.FogDensity);
	DynamicSkyMaterial->SetVectorParameterValue(TEXT("FogColor"), CurrentWeatherSettings.FogColor);
	DynamicSkyMaterial->SetScalarParameterValue(TEXT("TimeOfDay"), TimeOfDaySettings.CurrentHour / 24.0f);
}

EWeatherType ADynamicWeatherSystem::GenerateRandomWeather() const
{
	// Weight different weather types based on current weather
	TArray<EWeatherType> PossibleWeathers;
	
	switch (CurrentWeatherSettings.WeatherType)
	{
		case EWeatherType::Clear:
			PossibleWeathers = { EWeatherType::Clear, EWeatherType::Cloudy, EWeatherType::Fog };
			break;
		case EWeatherType::Cloudy:
			PossibleWeathers = { EWeatherType::Clear, EWeatherType::Cloudy, EWeatherType::LightRain, EWeatherType::Overcast };
			break;
		case EWeatherType::LightRain:
			PossibleWeathers = { EWeatherType::Cloudy, EWeatherType::LightRain, EWeatherType::HeavyRain };
			break;
		case EWeatherType::HeavyRain:
			PossibleWeathers = { EWeatherType::LightRain, EWeatherType::HeavyRain, EWeatherType::Storm };
			break;
		default:
			PossibleWeathers = { EWeatherType::Clear, EWeatherType::Cloudy, EWeatherType::LightRain };
			break;
	}

	int32 RandomIndex = FMath::RandRange(0, PossibleWeathers.Num() - 1);
	return PossibleWeathers[RandomIndex];
}
