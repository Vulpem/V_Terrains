#include "QuadTree.h"

#include "Application.h"
#include "ModuleRenderer3D.h"

QuadNode::QuadNode(float3 minPoint, float3 maxPoint)
	: m_parent(nullptr)
	, m_box(minPoint, maxPoint)
{
}

QuadNode::QuadNode(QuadNode* _parent)
	: m_parent(_parent)
	, m_box(m_parent->GetBox())
{
}

QuadNode::~QuadNode()
{
}

bool QuadNode::Add(GameObject* GO)
{
	if (m_box.Intersects(GO->m_aabb))
	{
		if (m_childs.empty() == true)
		{
			m_GOs.push_back(GO);

			if (m_GOs.size() > QUAD_GO_SIZE)
			{
				CreateChilds();
			}
		}
		else
		{
			std::vector<QuadNode*> collidedWith;
			for (std::vector<QuadNode>::iterator it = m_childs.begin(); it != m_childs.end(); it++)
			{
				if (it->m_box.Intersects(GO->m_aabb))
				{
					collidedWith.push_back(&*it);
				}
			}
			if (collidedWith.size() == 1)
			{
				collidedWith.front()->Add(GO);
			}
			else if (collidedWith.size() > 1)
			{
				m_GOs.push_back(GO);
			}
		}
		return true;
	}
	return false;
}

bool QuadNode::Remove(GameObject * GO)
{
	if (m_GOs.empty() == false)
	{
		for (std::vector<GameObject*>::iterator it = m_GOs.begin(); it != m_GOs.end(); it++)
		{
			if ((*it) == GO)
			{
				m_GOs.erase(it);
				Clean();
				return true;
			}
		}
	}

	if (m_childs.empty() == false)
	{
		for (std::vector<QuadNode>::iterator it = m_childs.begin(); it != m_childs.end(); it++)
		{
			if (it->Remove(GO))
			{
				return true;
			}
		}
	}
	return false;
}

void QuadNode::Draw() const
{
	float3 corners[8];
	m_box.GetCornerPoints(corners);
	App->m_renderer3D->DrawBox(corners);
	for (QuadNode child : m_childs)
	{
		child.Draw();
	}
}

void QuadNode::SetBox(int n, float3 breakPoint)
{
	AABB parentBox = m_parent->GetBox();
	switch (n)
	{
	case 0:
	{
		m_box.minPoint.x = parentBox.minPoint.x;
		m_box.minPoint.z = breakPoint.z;
		m_box.maxPoint.x = breakPoint.x;
		m_box.maxPoint.z = parentBox.maxPoint.z;
		break;
	}
	case 1:
	{
		m_box.minPoint.x = breakPoint.x;
		m_box.minPoint.z = breakPoint.z;
		m_box.maxPoint.x = parentBox.maxPoint.x;
		m_box.maxPoint.z = parentBox.maxPoint.z;
		break;
	}
	case 2:
	{
		m_box.minPoint.x = breakPoint.x;
		m_box.minPoint.z = parentBox.minPoint.z;
		m_box.maxPoint.x = parentBox.maxPoint.x;
		m_box.maxPoint.z = breakPoint.z;
		break;
	}
	case 3:
	{
		m_box.minPoint.x = parentBox.minPoint.x;
		m_box.minPoint.z = parentBox.minPoint.z;
		m_box.maxPoint.x = breakPoint.x;
		m_box.maxPoint.z = breakPoint.z;
		break;
	}
	}
}

void QuadNode::CreateChilds()
{
	float3 centerPoint = float3::zero;
	/*int n = 0;
	for (std::vector<GameObject*>::iterator it = m_GOs.begin(); it != m_GOs.end(); it++)
	{
		centerPoint.x += (*it)->aabb.CenterPoint().x;
		centerPoint.y += (*it)->aabb.CenterPoint().z;
		n++;
	}
	centerPoint /= n;

	
	//Checking if m_GOs collide with the X plane
	float2 newCenterPoint = centerPoint;
	for (std::vector<GameObject*>::iterator it = m_GOs.begin(); it != m_GOs.end(); it++)
	{
		while (Plane(float3(newCenterPoint.x, 0, newCenterPoint.y), float3(0, 0, 1)).Intersects((*it)->aabb) == true)
		{
			newCenterPoint.y++;
		}
	}
	//Checking if m_GOs collide with the Y plane
	for (std::vector<GameObject*>::iterator it = m_GOs.begin(); it != m_GOs.end(); it++)
	{
		while (Plane(float3(newCenterPoint.x, 0, newCenterPoint.y), float3(1, 0, 0)).Intersects((*it)->aabb) == true)
		{
			newCenterPoint.x++;
		}
	}

	if (newCenterPoint.x >= m_box.maxPoint.x || newCenterPoint.y >= m_box.maxPoint.z)
	{
		newCenterPoint.x = m_box.CenterPoint().x;
		newCenterPoint.y = m_box.CenterPoint().z;
	}*/
	float3 newCenterPoint = m_box.CenterPoint();

	for (int n = 0; n < 4; n++)
	{
		m_childs.push_back(QuadNode(this));
		m_childs.back().SetBox(n, newCenterPoint);
	}

	std::vector<GameObject*> tmp = m_GOs;
	m_GOs.clear();

	for (std::vector<GameObject*>::iterator it = tmp.begin(); it != tmp.end(); it++)
	{
		Add(*it);
	}
}

void QuadNode::Clean()
{
	bool childsHaveChilds = false;
	std::vector<GameObject*> childsGOs;
	for (std::vector<QuadNode>::iterator it = m_childs.begin(); it != m_childs.end(); it++)
	{
		if (it->m_childs.empty() == false)
		{
			//If a child has m_childs, we shouldn't erase any of them! Just in case
			childsHaveChilds = true;
			break;
		}
		for (std::vector<GameObject*>::iterator childIt = it->m_GOs.begin(); childIt != it->m_GOs.end(); childIt++)
		{
			childsGOs.push_back(*childIt);
		}
	}

	if (childsHaveChilds == false)
	{
		if (childsGOs.empty() == true)
		{
			m_childs.clear();
		}
		else if (childsGOs.size() + m_GOs.size() <= QUAD_GO_SIZE)
		{
			for (std::vector<GameObject*>::iterator it = childsGOs.begin(); it != childsGOs.end(); it++)
			{
				m_GOs.push_back(*it);
			}
			m_childs.clear();
		}

		if (m_parent != nullptr)
		{
			m_parent->Clean();
		}
	}
}


Quad_Tree::Quad_Tree(float3 minPoint, float3 maxPoint)
	: m_root(minPoint, maxPoint)
{
}

Quad_Tree::~Quad_Tree()
{
}

void Quad_Tree::Add(GameObject * GO)
{
	if (GO->m_aabb.IsFinite())
	{
		m_root.Add(GO);
	}
}

void Quad_Tree::Remove(GameObject * GO)
{
	if (GO->m_aabb.IsFinite())
	{
		m_root.Remove(GO);
	}
}


void Quad_Tree::Draw() const
{
	m_root.Draw();
}
