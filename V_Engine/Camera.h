#ifndef __CAMERA__
#define __CAMERA__

#include "Component.h"

enum FrustumCollision
{
	contains,
	intersects,
	outside
};

class Camera : public  Component
{
public:
	Camera(Gameobject* linkedTo);
	~Camera();

	void PreUpdate() override;

	void EditorContent() override;

	void SaveSpecifics(pugi::xml_node& myNode) override;
	void LoadSpecifics(pugi::xml_node& myNode) override;

	void PositionChanged() override;
	void UpdateCamMatrix();
	void UpdatePos();
	void UpdateOrientation();

	FrustumCollision Collides(AABB boundingBox);
	FrustumCollision Collides(float3 point);
	void SetCulling(bool culling) { m_forceDebugCulling = culling; }
	bool HasCulling() { return m_forceDebugCulling; }

	math::FrustumType SwitchViewType();

	const math::Frustum* GetFrustum() const { return &m_frustum; }

	ComponentType GetType() const override { return ComponentType::camera; }
    float3 GetPosition();

	void Draw(const ViewPort & port);
	void DrawFrustum();

	void SetHorizontalFOV(float horizontalFOV);
	void SetFarPlane(float farPlaneDistance);
	void SetNearPlane(float nearPlaneDistance);

	float4x4 GetViewMatrix();
	float4x4 GetProjectionMatrix();

	float m_aspectRatio = 1.4222222f;
	math::float3 m_positionOffset = float3::zero;
private:
	math::Frustum m_frustum;
	bool m_forceDebugCulling = false;
	float2 m_otherFOV = float2::zero;
};

#endif