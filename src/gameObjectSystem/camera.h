#pragma once
#ifndef __INCLUDE_GUARD_camera_h__
#define __INCLUDE_GUARD_camera_h__
#include "mathf.h"
#include "component.h"



/// <summary>
/// Default view direction is -z = transform->backward
/// </summary>
class Camera : public Component
{
private: // Members:
	float fovRadians;
	float aspectRatio;
	float nearClip;
	float farClip;
	Float4x4 projectionMatrix;
	bool updateProjectionMatrix;

public: // Methods:
	Camera();
	~Camera();

	// Setters:
	void SetFovDegrees(const float& fovDegrees);
	void SetFovRadians(const float& fovRadians);
	void SetAspectRatio(const float& aspectRatio);
	void SetNearClip(const float& nearClip);
	void SetFarClip(const float& farClip);

	// Getters:
	float GetFovDegrees() const;
	float GetFovRadians() const;
	float GetAspectRatio() const;
	float GetNearClip() const;
	float GetFarClip() const;
	Float4x4 GetViewMatrix() const;
	Float4x4 GetProjectionMatrix();

	// Overrides:
	std::string ToString() const override;

private: // Methods:
	void UpdateProjectionMatrix();
};



#endif // __INCLUDE_GUARD_camera_h__