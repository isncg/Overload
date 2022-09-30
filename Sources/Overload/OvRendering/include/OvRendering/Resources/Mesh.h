/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#pragma once

#include <vector>
#include <memory>

#include "OvRendering/Buffers/VertexArray.h"
#include "OvRendering/Buffers/IndexBuffer.h"
#include "OvRendering/Resources/IMesh.h"
#include "OvRendering/Geometry/Vertex.h"
#include "OvRendering/Geometry/BoundingSphere.h"

namespace OvRendering::Resources
{
	/**
	* Standard mesh of OvRendering
	*/
	class Mesh : public IMesh
	{
	public:
		/**
		* Create a mesh with the given vertices, indices and material index
		* @param p_vertices
		* @param p_indices
		* @param p_materialIndex
		*/
		Mesh();
		Mesh(std::vector<Geometry::Vertex>& p_vertices, const std::vector<uint32_t>& p_indices, uint32_t p_materialIndex);

		template<class T>
		void Init(std::vector<T>& p_vertices, const std::vector<uint32_t>& p_indices, uint32_t p_materialIndex);
		/**
		* Bind the mesh (Actually bind its VAO)
		*/
		virtual void Bind() override;

		/**
		* Unbind the mesh (Actually unbind its VAO)
		*/
		virtual void Unbind() override;

		/**
		* Returns the number of vertices
		*/
		virtual uint32_t GetVertexCount() override;

		/**
		* Returns the number of indices
		*/
		virtual uint32_t GetIndexCount() override;

		/**
		* Returns the material index of the mesh
		*/
		uint32_t GetMaterialIndex() const;

		/**
		* Returns the bounding sphere of the mesh
		*/
		const OvRendering::Geometry::BoundingSphere& GetBoundingSphere() const;

	private:
		template<class T>
		void CreateBuffers(std::vector<T>& p_vertices, const std::vector<uint32_t>& p_indices);
		template<class T>
		void ComputeBoundingSphere(std::vector<T>& p_vertices);

	private:
		uint32_t m_vertexCount;
		uint32_t m_indicesCount;
		uint32_t m_materialIndex;

		Buffers::VertexArray							m_vertexArray;
		std::unique_ptr<Buffers::VertexBuffer<float>>	m_vertexBuffer;
		std::unique_ptr<Buffers::IndexBuffer>			m_indexBuffer;

		Geometry::BoundingSphere m_boundingSphere;
	};

	template<class T>
	inline void Mesh::Init(std::vector<T>& p_vertices, const std::vector<uint32_t>& p_indices, uint32_t p_materialIndex)
	{
		m_vertexCount = static_cast<uint32_t>(p_vertices.size()),
		m_indicesCount = static_cast<uint32_t>(p_indices.size()),
		m_materialIndex = p_materialIndex;

		CreateBuffers(p_vertices, p_indices);
		ComputeBoundingSphere(p_vertices);
	}

	template<class T>
	inline void Mesh::CreateBuffers(std::vector<T>& p_vertices, const std::vector<uint32_t>& p_indices)
	{
		std::vector<float> vertexData;

		std::vector<unsigned int> rawIndices;

		for (auto& vertex : p_vertices)
		{
			vertexData.push_back(Geometry::VertexHelper::GetPosition(vertex).x);
			vertexData.push_back(Geometry::VertexHelper::GetPosition(vertex).y);
			vertexData.push_back(Geometry::VertexHelper::GetPosition(vertex).z);

			vertexData.push_back(Geometry::VertexHelper::GetUV(vertex).x);
			vertexData.push_back(Geometry::VertexHelper::GetUV(vertex).y);

			vertexData.push_back(Geometry::VertexHelper::GetNormal(vertex).x);
			vertexData.push_back(Geometry::VertexHelper::GetNormal(vertex).y);
			vertexData.push_back(Geometry::VertexHelper::GetNormal(vertex).z);

			vertexData.push_back(Geometry::VertexHelper::GetTangent(vertex).x);
			vertexData.push_back(Geometry::VertexHelper::GetTangent(vertex).y);
			vertexData.push_back(Geometry::VertexHelper::GetTangent(vertex).z);

			vertexData.push_back(Geometry::VertexHelper::GetBitangent(vertex).x);
			vertexData.push_back(Geometry::VertexHelper::GetBitangent(vertex).y);
			vertexData.push_back(Geometry::VertexHelper::GetBitangent(vertex).z);
		}

		m_vertexBuffer = std::make_unique<Buffers::VertexBuffer<float>>(vertexData);
		m_indexBuffer = std::make_unique<Buffers::IndexBuffer>(const_cast<uint32_t*>(p_indices.data()), p_indices.size());

		uint64_t vertexSize = sizeof(Geometry::Vertex);

		m_vertexArray.BindAttribute(0, *m_vertexBuffer, Buffers::EType::FLOAT, 3, vertexSize, 0);
		m_vertexArray.BindAttribute(1, *m_vertexBuffer, Buffers::EType::FLOAT, 2, vertexSize, sizeof(float) * 3);
		m_vertexArray.BindAttribute(2, *m_vertexBuffer, Buffers::EType::FLOAT, 3, vertexSize, sizeof(float) * 5);
		m_vertexArray.BindAttribute(3, *m_vertexBuffer, Buffers::EType::FLOAT, 3, vertexSize, sizeof(float) * 8);
		m_vertexArray.BindAttribute(4, *m_vertexBuffer, Buffers::EType::FLOAT, 3, vertexSize, sizeof(float) * 11);
	}
	
	template<class T>
	inline void Mesh::ComputeBoundingSphere(std::vector<T>& p_vertices)
	{
		m_boundingSphere.position = OvMaths::FVector3::Zero;
		m_boundingSphere.radius = 0.0f;

		if (!p_vertices.empty())
		{
			float minX = std::numeric_limits<float>::max();
			float minY = std::numeric_limits<float>::max();
			float minZ = std::numeric_limits<float>::max();

			float maxX = std::numeric_limits<float>::min();
			float maxY = std::numeric_limits<float>::min();
			float maxZ = std::numeric_limits<float>::min();

			for (auto& vertex : p_vertices)
			{
				minX = std::min(minX, Geometry::VertexHelper::GetPosition(vertex).x);
				minY = std::min(minY, Geometry::VertexHelper::GetPosition(vertex).y);
				minZ = std::min(minZ, Geometry::VertexHelper::GetPosition(vertex).z);

				maxX = std::max(maxX, Geometry::VertexHelper::GetPosition(vertex).x);
				maxY = std::max(maxY, Geometry::VertexHelper::GetPosition(vertex).y);
				maxZ = std::max(maxZ, Geometry::VertexHelper::GetPosition(vertex).z);
			}

			m_boundingSphere.position = OvMaths::FVector3{ minX + maxX, minY + maxY, minZ + maxZ } / 2.0f;

			for (auto& vertex : p_vertices)
			{
				const auto& position = Geometry::VertexHelper::GetPosition(vertex);
				m_boundingSphere.radius = std::max(m_boundingSphere.radius, OvMaths::FVector3::Distance(m_boundingSphere.position, position));
			}
		}
	}
}