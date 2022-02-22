#include "CDX12Sky.h"

namespace DX12
{
	void CDX12Sky::Render(bool basicGeometry) { CDX12GameObject::Render(basicGeometry); }
	void CDX12Sky::LoadNewMesh(std::string newMesh) { CDX12GameObject::LoadNewMesh(newMesh); }
}
