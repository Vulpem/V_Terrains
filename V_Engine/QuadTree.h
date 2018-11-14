#ifndef __QUADTREE__
#define __QUADTREE__

#include "GameObject.h"

#define QUAD_GO_SIZE 1

class QuadNode
{
public:
	//Just for the root node!
	QuadNode(float3 minPoint, float3 maxPoint);
	//Default constructor
	QuadNode(QuadNode* parent);
	~QuadNode();

	/*Try to add a GO to this Node.
	Won't be added and return false if the object's m_aabb doesn't collide with this node
	Will return true if it's added*/
	bool Add(GameObject* GO);
	bool Remove(GameObject* GO);

	template <typename C>
	std::vector<GameObject*> FilterCollisions(C col);

	void Draw() const;

	AABB GetBox() { return m_box; }
	void SetBox(int n, float3 breakPoint);

private:
	void CreateChilds();
	void Clean();

private:
	QuadNode* m_parent;
	std::vector<QuadNode> m_childs;
	AABB m_box;

	std::vector<GameObject*> m_GOs;
};


class Quad_Tree
{
public:
	Quad_Tree(float3 minPoint, float3 maxPoint);
	~Quad_Tree();

	void Add(GameObject* GO);
	void Remove(GameObject* GO);

	template <typename c>
	std::vector<GameObject*> FilterCollisions(c col);

	void Draw() const;
private:
	QuadNode m_root;
};

//QuadNode
template<typename C>
inline std::vector<GameObject*> QuadNode::FilterCollisions(C col)
{
	std::vector<GameObject*> ret;
	if (m_box.Intersects(col))
	{
		if (m_GOs.empty() == false)
		{
			for (auto go : m_GOs)
			{
				AABB goAABB = go->GetAABB();
				if (go->IsActive() && goAABB.IsFinite() && col.Intersects(goAABB) == true)
				{
					ret.push_back(go);
				}
			}
		}
		if (m_childs.empty() == false)
		{
			for (auto child : m_childs)
			{
				std::vector<GameObject*> toAdd = child.FilterCollisions(col);
				for (auto goToAdd : toAdd)
				{
					ret.push_back(goToAdd);
				}
			}
		}
	}
	return ret;
}


//QuadTree
template<typename c>
inline std::vector<GameObject*> Quad_Tree::FilterCollisions(c col)
{
	return m_root.FilterCollisions(col);
}

#endif // !__QUADTREE__