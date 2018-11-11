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

	void SaveSpecifics(pugi::xml_node& myNode);
	void LoadSpecifics(pugi::xml_node& myNode);

	math::float4x4 GetLocalTransformMatrix();

	void UpdateGlobalTransform();
	math::float4x4 GetGlobalTransform();

	void UpdateEditorValues();

	void SetLocalPos(float x, float y, float z);
	void SetLocalPos(float3 pos);
	math::float3 GetLocalPos();

	void SetGlobalPos(float x, float y, float z);
	void SetGlobalPos(float3 pos);
	math::float3 GetGlobalPos();
	void Translate(float x, float y, float z);
	void Translate(float3 m);

	void SetLocalRot(float x, float y, float z);
	void SetLocalRot(float3 rot);
	void SetLocalRot(float x, float y, float z, float w);
	math::float3 GetLocalRot();
	Quat GetLocalRotQuat();

	void SetGlobalRot(float x, float y, float z);
	void SetGlobalRot(float3 rotation);
	void RotateLocal(float3 rotation);
	math::Quat GetGlobalRotQuat();
	math::float3 GetGlobalRot();

	void SetLocalScale(float x, float y, float z);
	math::float3 GetLocalScale();

	//void SetGlobalScale(float x, float y, float z);
	math::float3 GetGlobalScale();

	void LookAt(const float3 &Spot, float3 worldUp = float3(0,1,0));

	float3 Up();
	float3 Down();
	float3 Left();
	float3 Right();
	float3 Forward();
	float3 Backward();

	static float3 WorldUp();
	static float3 WorldDown();
	static float3 WorldLeft();
	static float3 WorldRight();
	static float3 WorldForward();
	static float3 WorldBackward();

	GameObject* GetGameobject();

	void SetParent(Transform* parent);
	Transform* GetParent();

	void AddChild(Transform* newChild);
	std::vector<Transform*> GetChilds();

	void Draw(const ViewPort & port);
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