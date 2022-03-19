# RenderingEngine

This project proposes a DirectX based real time rendering engine with a scene manager. Submitted as 3rd year project at University of Central Lancashire.

Supports: DirectX11 and DirectX12 (currently in development)
![projectScreen](https://user-images.githubusercontent.com/55553007/157924155-b7834d93-874a-4498-8be8-bbfed3515c5d.png)

# Features
### Scene manager
XML driven importer.
Can import and export scenes, it will save the object's position, rotation and scale, plus the mesh and the textures.

### Renderer
- Lighting (Simple, Directional, Spot, Omnidirectional lights)
- PBR
- Real time Cube Reflection Map
- Post processing (SSAO, Chromatic aberration, God Rays, Blur, Bloom and others)

![projectScreen2](https://user-images.githubusercontent.com/55553007/157924246-dc9357d8-13aa-4d00-98aa-f6db986bca43.png)

### Dependencies
- ImGui (Docking)
- ImGuizmo
- ImGuiFileExplorer
- Assimp
- TinyXML
- DirectXTK

### How to
- Download Media folder [Here](https://msuclanac-my.sharepoint.com/:f:/g/personal/ascapillati_uclan_ac_uk/EqRhVGrRGaNFtFnCQXclRZMBth0r8Dwb7IT48iVw3P1jbg?e=BY6D1M) and place inside the main folder
- Open the .sln with Visual Studio
- Let me know if it doesn't work

### Future updates
- Raytracing 

### Poster
[PosterScapillatiAlex.pdf](https://github.com/AlexScapillati/RenderingEngineRaytracing/files/8232730/PosterScapillatiAlex.pdf)
