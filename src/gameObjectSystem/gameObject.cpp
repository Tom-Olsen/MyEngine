#include "gameObject.h"
#include "emberEngine.h"



// Constructors/Destructor:
GameObject::GameObject(const std::string& name)
{
	m_name = name;
	isActive = true;
	m_pTransform = new Transform(Float3(0.0f), Float3x3::identity, Float3(1.0f));
	this->m_pScene = nullptr;
	AddComponent(m_pTransform);
}
GameObject::~GameObject()
{

}



// Public methods:
// Setters:
void GameObject::SetScene(Scene* pScene)
{
	m_pScene = pScene;
	for (auto& [_, component] : m_components)
		component->SetScene(pScene);
}
template <typename T>
void GameObject::AddComponent(T* pComponent)
{
	// Check if T is a Component:
    static_assert(std::is_base_of<Component, T>::value, "T is not a Component.");

	// Check if component is already attached to a gameObject:
	if (pComponent->GetGameObject() != nullptr)
	{
		LOG_WARN("Component '{}' is already attached to a gameObject!", std::string(typeid(T).name()));
		return;
	}

	// Add component to gameObject if not already attached:
	if (m_components.find(typeid(T)) == m_components.end())
	{
		m_components.emplace(typeid(T), std::unique_ptr<T>(pComponent));
		pComponent->SetGameObject(this);
		pComponent->SetTransform(m_pTransform);
	}
	else
		LOG_WARN("GameObject '{}' already owns a '{}' component!", m_name, std::string(typeid(T).name()));
}

// Getters:
const std::string& GameObject::GetName() const
{
	return m_name;
}
Transform* const GameObject::GetTransform() const
{
	return m_pTransform;
}
Scene* const GameObject::GetScene() const
{
	return m_pScene;
}
template <typename T>
T* GameObject::GetComponent()
{
	static_assert(std::is_base_of<Component, T>::value, "T is not a Component.");
	auto it = m_components.find(typeid(T));
	if (it != m_components.end())
		return dynamic_cast<T*>(it->second.get());
	return nullptr;
}
const std::unordered_map<std::type_index, std::unique_ptr<Component>>& GameObject::GetComponents() const
{
	return m_components;
}

// Debugging:
void GameObject::PrintComponents() const
{
	LOG_TRACE("Components attached to: {}", m_name);
	for (const auto& pair : m_components)
		LOG_TRACE(pair.second->ToString());
}



// Explicit instantiations:
template void GameObject::AddComponent(Camera* component);
template void GameObject::AddComponent(CameraController* component);
template void GameObject::AddComponent(DirectionalLight* component);
template void GameObject::AddComponent(DrawMeshData* component);
template void GameObject::AddComponent(MeshRenderer* component);
template void GameObject::AddComponent(PointLight* component);
template void GameObject::AddComponent(RotationController* component);
template void GameObject::AddComponent(SpinGlobal* component);
template void GameObject::AddComponent(SpinLocal* component);
template void GameObject::AddComponent(SpotLight* component);
template void GameObject::AddComponent(Transform* component);
template Camera*				GameObject::GetComponent<Camera>();
template CameraController*		GameObject::GetComponent<CameraController>();
template DirectionalLight*		GameObject::GetComponent<DirectionalLight>();
template DrawMeshData*			GameObject::GetComponent<DrawMeshData>();
template MeshRenderer*			GameObject::GetComponent<MeshRenderer>();
template PointLight*			GameObject::GetComponent<PointLight>();
template RotationController*	GameObject::GetComponent<RotationController>();
template SpinGlobal*			GameObject::GetComponent<SpinGlobal>();
template SpinLocal*				GameObject::GetComponent<SpinLocal>();
template SpotLight*				GameObject::GetComponent<SpotLight>();
template Transform*				GameObject::GetComponent<Transform>();