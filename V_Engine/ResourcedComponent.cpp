#include "ResourcedComponent.h"

#include "Application.h"


void ResourcedComponent::LoadResource(std::string resourceToLoad)
{
	if (m_resource != 0)
	{
		UnLinkResource();
	}
	LinkResource(resourceToLoad);
}

void ResourcedComponent::LinkResource(std::string resName)
{
	UnLinkResource();
	m_resource = App->m_resourceManager->LinkResource(resName, GetType());
}

void ResourcedComponent::UnLinkResource()
{
	if (m_resource != 0)
	{
		App->m_resourceManager->UnlinkResource(m_resource);
		m_resource = 0;
	}
}
