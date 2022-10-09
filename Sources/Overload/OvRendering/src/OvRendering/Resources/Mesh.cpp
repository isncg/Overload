/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#include <algorithm>

#include "OvRendering/Resources/Mesh.h"

OvRendering::Resources::Mesh::Mesh()
{
}

OvRendering::Resources::Mesh::Mesh(std::vector<Geometry::Vertex>& p_vertices, const std::vector<uint32_t>& p_indices, uint32_t p_materialIndex, bool hasBone) :
	m_vertexCount(static_cast<uint32_t>(p_vertices.size())),
	m_indicesCount(static_cast<uint32_t>(p_indices.size())),
	m_materialIndex(p_materialIndex)
{
	CreateBuffers(p_vertices, p_indices, hasBone);
	ComputeBoundingSphere(p_vertices);
}

void OvRendering::Resources::Mesh::Bind()
{
	m_vertexArray.Bind();
}

void OvRendering::Resources::Mesh::Unbind()
{
	m_vertexArray.Unbind();
}

uint32_t OvRendering::Resources::Mesh::GetVertexCount()
{
	return m_vertexCount;
}

uint32_t OvRendering::Resources::Mesh::GetIndexCount()
{
	return m_indicesCount;
}

uint32_t OvRendering::Resources::Mesh::GetMaterialIndex() const
{
	return m_materialIndex;
}

const OvRendering::Geometry::BoundingSphere& OvRendering::Resources::Mesh::GetBoundingSphere() const
{
	return m_boundingSphere;
}