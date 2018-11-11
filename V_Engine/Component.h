#ifndef __COMPONENT__
#define __COMPONENT__

#include "Globals.h"
#include "Math.h"

#include "PugiXml\src\pugixml.hpp"

class GameObject;
class ViewPort;

enum class ComponentType
{
	GO = 1,
	mesh,
	material,
	texture,
	camera,
	billboard,
	shader,
	//Keep this "C_None" always last
	none
};

class Component
{
public:
	Component(GameObject* linkedTo, ComponentType type);
	virtual ~Component();
	
	virtual void Enable();
	virtual void Disable();

	virtual void PreUpdate() {};
	virtual void Update() {};
	virtual void PostUpdate() {};
	virtual void Draw(const ViewPort & port) {};
	virtual void DrawOnEditor();

	virtual ComponentType GetType() const = 0;
	bool IsEnabled() { return m_enabled; }
	virtual bool MissingComponent() { return false; }

	void Save(pugi::xml_node& myNode);
	GameObject* GetOwner() const;
	bool MarkedForDeletion() const;

	virtual void LoadSpecifics(pugi::xml_node& myNode) {}

	const uint64_t GetUID() { return m_uid; }
	void Delete();

	//For system use, do not call
	bool TryDeleteNow();

	std::string m_name;
protected:
	virtual void SaveSpecifics(pugi::xml_node& myNode) {}

	virtual void OnEnable() {}
	virtual void OnDisable() {}

	virtual void EditorContent() {};

	ComponentType m_type;
	uint64_t m_uid;
	bool m_toDelete = false;
	GameObject* m_gameObject;
private:
	bool m_enabled = true;
};

#endif