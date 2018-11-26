#pragma once
#include "Globals.h"
#include "GameObject.h"
#include "QuadTree.h"

class Scene
{
public:
	Scene(std::string name);
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
	~Scene();

	void Render(const ViewPort& port) const;

	void AppendGameobject(Gameobject* toAppend);
	std::vector<Gameobject*> GetGameobjects() const;

	template <typename C>
	std::vector<Gameobject*> FilterCollisions(C col) const;
	std::map<float, Gameobject*> FilterOrderedCollisions(const LineSegment& ray) const;
	bool RayCast(const LineSegment& ray, Gameobject*& OUT_gameobject, float3& OUT_position, float3& OUT_normal, bool collideWithAABBs = false);

	std::string m_name;
	bool m_renderQuadtree = false;
private:
	void RenderGos(const ViewPort& port) const;

	Gameobject m_root;
	Quadtree m_quadTree;
	std::vector<Gameobject*> m_dynamicGos;
};




// Returns objects whose AABB collides with the recieved collision object
template<typename C>
inline std::vector<Gameobject*> Scene::FilterCollisions(C col) const
{
	std::vector<Gameobject*> ret = m_quadTree.FilterCollisions(col);
	for (auto go : m_dynamicGos)
	{
		AABB goAABB = go->GetAABB();
		if (go->IsActive() && goAABB.IsFinite() && goAABB.Intersects(col))
		{
			ret.push_back(go);
		}
	}
	return ret;
}
