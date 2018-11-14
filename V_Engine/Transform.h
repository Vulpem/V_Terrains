#ifndef __TRANSFORM__
#define __TRANSFORM__

#include "Component.h"


class math::float3;
class math::Quat;

class Transform
{
public:
	Transform(GameObject* linkedTo);
	~Transform();

	void SaveSpecifics(pugi::xml_node& myNode) const;
	void LoadSpecifics(pugi::xml_node& myNode);

	math::float4x4 GetLocalTransformMatrix() const;

	void UpdateGlobalTransform();
	math::float4x4 GetGlobalTransform() const;

	void UpdateEditorValues();

	void SetLocalPos(float x, float y, float z);
	void SetLocalPos(float3 pos);
	math::float3 GetLocalPos() const;

	void SetGlobalPos(float x, float y, float z);
	void SetGlobalPos(float3 pos);
	math::float3 GetGlobalPos() const;
	void Translate(float x, float y, float z);
	void Translate(float3 m);

	void SetLocalRot(float x, float y, float z);
	void SetLocalRot(float3 rot);
	void SetLocalRot(float x, float y, float z, float w);
	math::float3 GetLocalRot() const;
	Quat GetLocalRotQuat() const;

	void SetGlobalRot(float x, float y, float z);
	void SetGlobalRot(float3 rotation);
	void RotateLocal(float3 rotation);
	math::Quat GetGlobalRotQuat() const;
	math::float3 GetGlobalRot() const;

	void SetLocalScale(float x, float y, float z);
	math::float3 GetLocalScale() const;

	//void SetGlobalScale(float x, float y, float z);
	math::float3 GetGlobalScale() const;

	void LookAt(const float3 &Spot, float3 worldUp = float3(0,1,0));

	float3 Up() const;
	float3 Down() const;
	float3 Left() const;
	float3 Right() const;
	float3 Forward() const;
	float3 Backward() const;

	static float3 WorldUp();
	static float3 WorldDown();
	static float3 WorldLeft();
	static float3 WorldRight();
	static float3 WorldForward();
	static float3 WorldBackward();

	GameObject* GetGameobject() const;

	void SetParent(Transform* parent);
	Transform* GetParent() const;

	void AddChild(Transform* newChild);
	std::vector<Transform*> GetChilds() const;

	void Draw(const ViewPort & port) const;
	void EditorContent();

	bool m_allowRotation = true;
private:

	math::Quat m_localRotation = math::Quat::identity;
	math::float3 m_localPosition = math::float3::zero;
	math::float3 m_localScale = math::float3::zero;
	//TMP - Needed to fix the attribute editor display
	math::float3 m_editorRot = math::float3::zero;
	math::float3 m_editorGlobalRot = math::float3::zero;

	math::float4x4 m_globalTransform = math::float4x4::identity;

	GameObject* m_gameObject;

	std::vector<Transform*> m_childs;
	Transform* m_parent = nullptr;
};

#endif