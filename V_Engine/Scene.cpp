#include "Scene.h"
#include "Mesh.h"

Scene::Scene()
	: m_root()
	, m_quadTree
	(	float3(WORLD_WIDTH / -2, WORLD_HEIGHT / -2, WORLD_DEPTH / -2)
	,	float3(WORLD_WIDTH / 2, WORLD_HEIGHT / 2, WORLD_DEPTH / 2))
{
	m_root.SetName("Root");
	m_root.GetTransform()->SetParent(nullptr);
}

Scene::~Scene()
{
}

void Scene::Render(const ViewPort & port) const
{
}

void Scene::AppendGameobject(Gameobject * toAppend)
{
	toAppend->GetTransform()->SetParent(m_root.GetTransform());
}

std::vector<Gameobject*> Scene::GetGameobjects() const
{
	std::vector<Transform*> transforms = m_root.GetTransform()->GetChilds();
	std::vector<Gameobject*> gameobjects;
	gameobjects.reserve(transforms.size());
	std::for_each(transforms.begin(), transforms.end(),
		[&gameobjects](Transform* transform)
	{ gameobjects.push_back(transform->GetGameobject()); });
	return gameobjects;
}

bool Scene::RayCast(const LineSegment & ray, Gameobject *& OUT_gameobject, float3& OUT_position, float3& OUT_normal, bool collideWithAABBs)
{
	TIMER_RESET_STORED("Raycast");
	TIMER_START("Raycast");
	OUT_position = float3::zero;
	OUT_normal = float3::zero;
	OUT_gameobject = nullptr;

	float3 collisionPosition = float3::zero;
	float3 collisionNormal = float3::zero;
	std::map<float, Gameobject*> orderedCollisions;

	std::vector<Gameobject*> aabbCollisions = FilterCollisions(ray);
	//Ordering all the collisions by stuffing them into a map
	for (auto go : aabbCollisions)
	{
		float nearDistance;
		float farDistance;
		//The distance is normalized between [0,1] and is the relative position in the Segment the AABB collides
		if (go->GetOBB().Intersects(ray, nearDistance, farDistance) == true)
		{
			orderedCollisions.insert(std::pair<float, Gameobject*>(MIN(nearDistance, farDistance), go));
		}
	}

	if (collideWithAABBs == false)
	{
		for (auto go : orderedCollisions)
		{
			float collisionDistance = floatMax;
			if (go.second->HasComponent<Mesh>())
			{
				std::vector<Mesh*> meshes;
				meshes.reserve(3);
				go.second->GetComponents<Mesh>(meshes);
				for (auto mesh : meshes)
				{
					if (mesh->RayCast(ray, &collisionPosition, &collisionNormal))
					{
						OUT_position = collisionPosition;
						OUT_normal = collisionNormal;
						OUT_gameobject = go.second;
					}
				}
				if (OUT_gameobject != nullptr)
				{
					TIMER_READ_MS("Raycast");
					return true;
				}
			}
		}
	}
	else if (orderedCollisions.empty() == false)
	{
		OUT_gameobject = orderedCollisions.begin()->second;
		OUT_position = ray.a + ray.Dir() * orderedCollisions.begin()->first;
		//TODO: fix raycast normal when checking against AABBs
		OUT_normal = float3::zero;
		TIMER_READ_MS("Raycast");
		return true;
	}

	TIMER_READ_MS("Raycast");
	return false;
}