#pragma once

#include "DX12GameObject.h"

#include "../../Math/CVector3.h"

namespace DX12
{
	class CDX12Sky : virtual public CDX12GameObject, public CSky
	{
		~CDX12Sky() override = default;

		public:
			explicit CDX12Sky(CDX12GameObject& cdx12GameObject)
				: CDX12GameObject(cdx12GameObject)
			{
			}

			CDX12Sky(CDX12Engine* engine, const std::string& mesh, const std::string& name, const std::string& diffuseMap, const CVector3& position, const CVector3& rotation, float scale)
				: CDX12GameObject(engine, mesh, name, diffuseMap, position, rotation, scale)
			{
			}

			CDX12Sky(CDX12Engine* engine, const std::string& id, const std::string& name, const CVector3& position, const CVector3& rotation, float scale)
				: CDX12GameObject(engine, id, name, position, rotation, scale)
			{
			}

			void Render(bool basicGeometry = false) override;
	};
}
