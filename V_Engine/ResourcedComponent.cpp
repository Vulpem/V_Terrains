#include "ResourcedComponent.h"

#include "Application.h"


void ResourcedComponent::LinkResource(std::string resName)
{
	UnLinkResource();
	resource = App->m_resourceManager->LinkResource(resName, GetType());
}

void ResourcedComponent::UnLinkResource()
{
	if (resource != 0)
	{
		App->m_resourceManager->UnlinkResource(resource);
		resource = 0;
	}
}
