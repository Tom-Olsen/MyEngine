#include "gameObject.h"
#include "scene.h"
#include "logger.h"



// Constructors:
GameObject::GameObject(std::string name)
{
	this->name = name;
	this->isActive = true;
 	transform = new Transform(Float3(0.0f), Float3x3::identity, Float3(1.0f));
	AddComponent(transform);
}



// Destructor:
GameObject::~GameObject()
{

}



// Public:
void GameObject::SetScene(Scene* scene)
{
	this->scene = scene;
	for (auto& [_, component] : components)
		component->scene = scene;
}
template <typename T>
void GameObject::AddComponent(T* component)
{
	// Check if T is a Component:
    static_assert(std::is_base_of<Component, T>::value, "T is not a Component.");

	// Check if component is already attached to a gameObject:
	if (component->gameObject != nullptr)
	{
		LOG_WARN("Component '{}' is already attached to a gameObject!", std::string(typeid(T).name()));
		return;
	}

	// If gameObject aleady owns a component of type T, do nothing.
	if (components.emplace(typeid(T), std::unique_ptr<T>(component)).second == false)
	{
		LOG_WARN("GameObject '{}' already owns a '{}' component!", name, std::string(typeid(T).name()));
		return;
	}
	else
	{
		component->gameObject = this;
		component->transform = transform;
	}
}

template <typename T>
T* GameObject::GetComponent()
{
	static_assert(std::is_base_of<Component, T>::value, "T is not a Component.");
	auto it = components.find(typeid(T));
	if (it != components.end())
		return dynamic_cast<T*>(it->second.get());
	return nullptr;
}

void GameObject::PrintComponents() const
{
	LOG_TRACE("Components attached to: {}", name);
	for (const auto& pair : components)
		LOG_TRACE(pair.second->ToString());
}



// Explicit instantiations:
template void GameObject::AddComponent(Transform* component);
template void GameObject::AddComponent(Camera* component);
template void GameObject::AddComponent(CameraController* component);
template void GameObject::AddComponent(MeshRenderer* component);
template void GameObject::AddComponent(SpinLocal* component);
template void GameObject::AddComponent(SpinGlobal* component);
template void GameObject::AddComponent(DirectionalLight* component);
template void GameObject::AddComponent(SpotLight* component);
template void GameObject::AddComponent(PointLight* component);
template void GameObject::AddComponent(DrawMeshData* component);
template Transform* GameObject::GetComponent<Transform>();
template Camera* GameObject::GetComponent<Camera>();
template CameraController* GameObject::GetComponent<CameraController>();
template MeshRenderer* GameObject::GetComponent<MeshRenderer>();
template SpinLocal* GameObject::GetComponent<SpinLocal>();
template SpinGlobal* GameObject::GetComponent<SpinGlobal>();
template DirectionalLight* GameObject::GetComponent<DirectionalLight>();
template SpotLight* GameObject::GetComponent<SpotLight>();
template PointLight* GameObject::GetComponent<PointLight>();
template DrawMeshData* GameObject::GetComponent<DrawMeshData>();