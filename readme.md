This repository contains a custom render engine developed using DirectX. The engine implements several advanced rendering features, including:

Deferred Rendering: Efficiently calculates light
Instance Rendering: Efficiently handles multiple instances of the same object, optimizing rendering performance for scenes with repeated assets.
Physically Based Rendering (PBR): Achieves realistic material appearances by simulating how light interacts with different surfaces based on physical properties.
Image-Based Lighting (IBL): Utilizes environment maps to provide realistic lighting and reflections, enhancing the visual quality of scenes.
Area Lighting: Supports more realistic light sources, such as rectangular lights, to create softer and more accurate shadows.
GPU Particles: Implements particle systems that are computed on the GPU for high-performance simulations of effects like smoke, fire, and explosions.
Volumetric Fog: Creates atmospheric effects with light scattering and absorption, adding depth and realism to the environment.
Decal Rendering: Allows the dynamic projection of textures onto surfaces, useful for adding details like bullet holes, cracks, and stains.
The engine demonstrates a deep understanding of modern graphics programming techniques and optimizations for real-time rendering. The codebase includes detailed implementations and optimizations for CPU-GPU interaction, profiling, and performance improvements.
