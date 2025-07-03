# Ecosystem Sandbox

Ecosystem Sandbox is a real-time ecosystem simulation sandbox template combining custom implementations of genetic evolution, emergent AI behavior, Chaos physics, and Nanite-rendered environments for a fully dynamic and destructible world.

## Key Features

### Genetic Algorithm System
- **Fitness-based Evolution**: Creatures evolve based on survival, reproduction success, resource gathering, and combat effectiveness
- **Trait System**: Each creature has genetic traits including:
  - Physical: Size, Speed, Strength, Endurance
  - Behavioral: Movement Type, Aggression Level, Reproduction Rate, Lifespan
  - Intelligence: Intelligence, Vision Range, Social Tendency
  - Survival: Hunger Resistance, Damage Resistance
  - Appearance: Primary and Secondary Colors

### AI Behavior System
- **Behavior Trees**: Individual AI driven by behavior trees for complex decision making
- **AI Perception**: Sight and hearing systems for environmental awareness
- **State Management**: Creatures have multiple states (Idle, Wandering, Seeking, Feeding, Fleeing, Fighting, Mating, Dying)
- **Emergent Behaviors**: Complex interactions emerge from simple rules

### Chaos Physics Integration
- **Full Physics Interaction**: Creatures push, fight, and collide destructively
- **Semi-Destructible Environment**: Trees fall, terrain can be deformed
- **Realistic Physics**: Mass, impulse, and collision systems affect creature interactions

### Nanite Environment System
- **High-Detail Terrain**: Dense, high-detail terrain with no LOD loss
- **Optimized Foliage**: Trees, bushes, grass, rocks, and flowers all Nanite-optimized
- **Dynamic Destruction**: Environment elements can be destroyed and regenerated
- **Resource System**: Creatures can consume foliage for nutrients

### Lumen Global Illumination
- **Real-Time GI**: Dynamic global illumination responds to lighting changes
- **Realistic Reflections**: Lumen reflections for accurate environmental lighting
- **Weather-Responsive Lighting**: Lighting adapts to weather conditions automatically
- **Sky Light Capture**: Real-time sky light capture for dynamic outdoor lighting

### Dynamic Weather System
- **8 Weather Types**: Clear, Cloudy, Overcast, Light Rain, Heavy Rain, Storm, Fog, Snow
- **Time of Day Cycle**: Dynamic day/night cycle affecting creature behavior
- **Weather Effects**: Rain, snow, fog with Niagara particle systems
- **Environmental Impact**: Weather affects creature movement, energy, and visibility
- **Atmospheric Lighting**: Weather conditions dynamically affect Lumen lighting

### Population Management
- **Generation Tracking**: Monitor population statistics across generations
- **Automatic Evolution**: Populations automatically advance to new generations
- **Data Collection**: Comprehensive tracking of fitness scores, traits, and survival data

## Project Structure

```
Source/EcosystemSandbox/
├── Public/
│   ├── CreatureGenetics.h          # Genetic algorithm and trait system
│   ├── Creature.h                  # Main creature class with physics
│   ├── CreatureAIController.h      # AI controller with behavior trees
│   ├── EcosystemManager.h          # Population and evolution management
│   ├── NaniteEnvironment.h         # Nanite-optimized environment
│   ├── DynamicWeatherSystem.h      # Lumen GI with dynamic weather
│   ├── BTTask_FindFood.h           # Behavior tree task for food seeking
│   └── EcosystemSandboxGameMode.h  # Main game mode
└── Private/
    ├── CreatureGenetics.cpp
    ├── Creature.cpp
    ├── CreatureAIController.cpp
    ├── EcosystemManager.cpp
    ├── NaniteEnvironment.cpp
    ├── DynamicWeatherSystem.cpp
    ├── BTTask_FindFood.cpp
    └── EcosystemSandboxGameMode.cpp
```

## Core Systems

### Genetic Algorithm
The genetic system implements:
- **Crossover**: Offspring inherit traits from two parents
- **Mutation**: Random changes to traits with configurable rates
- **Selection**: Fitness-based parent selection for next generation
- **Fitness Calculation**: Multi-factor fitness scoring system

### Behavior Tree Tasks
- **Find Food**: Locates nearby nutrient sources
- **Avoid Predators**: Escape from aggressive creatures
- **Seek Mates**: Find compatible partners for reproduction
- **Territorial Defense**: Protect claimed areas

### Environment Interaction
- **Destructible Foliage**: Trees and plants can be destroyed
- **Nutrient System**: Creatures gain energy from consuming plants
- **Regrowth Mechanics**: Destroyed vegetation regenerates over time
- **Physics Deformation**: Impact forces affect the environment

### Weather & Lighting Effects
- **Dynamic Lumen Lighting**: Global illumination adapts to weather and time of day
- **Weather Impact on Creatures**: Temperature affects energy consumption and movement
- **Visibility Modifiers**: Fog and rain reduce creature perception and navigation
- **Atmospheric Scattering**: Realistic sky and fog rendering with weather transitions
- **Real-Time Reflections**: Lumen reflections update with changing weather conditions

## Configuration

### Ecosystem Settings
- `InitialPopulation`: Starting number of creatures (default: 50)
- `MaxPopulation`: Maximum creatures allowed (default: 200)
- `MinPopulation`: Minimum before generation advance (default: 10)
- `GenerationTimeLimit`: Max time per generation (default: 300s)

### Genetic Algorithm Parameters
- `MutationRate`: Probability of trait mutation (default: 0.1)
- `MutationStrength`: Magnitude of mutations (default: 0.2)
- `ElitePercentage`: Top performers carried to next gen (default: 0.2)

### Environment Parameters
- `EnvironmentSize`: Size of the ecosystem area (default: 10000)
- `FoliageConfigs`: Types and distributions of vegetation
- `RegrowthRate`: Speed of vegetation regeneration (default: 1.0)

## Building and Running

### Prerequisites
- Unreal Engine 5.4 or later
- Chaos Physics plugin enabled
- Nanite support enabled

### Build Steps
1. Clone the repository
2. Right-click `EcosystemSandbox.uproject` and select "Generate Visual Studio project files"
3. Open `EcosystemSandbox.sln` in Visual Studio
4. Build the solution (Development Editor configuration)
5. Launch Unreal Editor
6. Create a new level or use the provided sample map
7. Add an `EcosystemSandboxGameMode` to your level
8. Run the simulation

## Usage

### Starting a Simulation
1. Place an `EcosystemManager` actor in your level
2. Place a `NaniteEnvironment` actor for the terrain and foliage
3. Configure the ecosystem parameters in the manager
4. Start Play-in-Editor or package for standalone execution

### Monitoring Evolution
- Use the `PopulationStats` structure to track:
  - Current generation number
  - Average and maximum fitness scores
  - Trait distributions across population
  - Dominant genetic characteristics

### Analyzing Results
- Generation data is logged for analysis
- Fitness scores track survival effectiveness
- Trait distributions show evolutionary trends
- Visual differences in creature appearance reflect genetic diversity

## Advanced Features

### Custom Behavior Trees
Create custom behavior tree tasks by inheriting from `UBTTaskNode`:
```cpp
UCLASS()
class UBTTask_CustomBehavior : public UBTTaskNode
{
    GENERATED_BODY()
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
```

### Custom Genetic Traits
Extend the `FCreatureGenes` structure to add new traits:
```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite)
float NewTrait = 1.0f;
```

### Environment Customization
Add new foliage types to `FNaniteFoliageConfig`:
```cpp
FNaniteFoliageConfig NewFoliage;
NewFoliage.FoliageType = EFoliageType::NewType;
NewFoliage.NaniteMesh = LoadObject<UStaticMesh>(...);
```

## Performance Considerations

### Nanite Optimization
- All environment meshes should be Nanite-enabled
- Use Nanite for high-detail, static geometry
- Avoid Nanite for frequently animated objects

### Chaos Physics
- Limit concurrent physics interactions
- Use physics sub-stepping for stability
- Optimize collision meshes for performance

### Population Scaling
- Monitor frame rate with large populations
- Consider LOD systems for distant creatures
- Use object pooling for creature spawning

## Troubleshooting

### Common Issues
- **Build Errors**: Ensure all dependencies are installed and Chaos Physics is enabled
- **Performance Issues**: Reduce initial population or environment complexity
- **AI Not Working**: Check behavior tree assets are properly assigned
- **Physics Problems**: Verify Chaos Physics is enabled in project settings

### Debug Commands
```cpp
// Log population statistics
UE_LOG(LogTemp, Warning, TEXT("Population: %d, Generation: %d"), 
    CurrentStats.TotalPopulation, CurrentStats.Generation);

// Force generation advancement
EcosystemManager->AdvanceGeneration();

// Reset simulation
EcosystemManager->ResetEcosystem();
```

## Contributing

This project serves as a foundation for ecosystem simulation research and game development. Key areas for expansion:
- Additional creature behaviors
- More complex environmental interactions
- Advanced genetic algorithms
- Machine learning integration
- Multi-species ecosystems
