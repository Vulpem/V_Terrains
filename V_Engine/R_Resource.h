#ifndef  __R_RESOURCE__
#define __R_RESOURCE__

#include "Globals.h"
#include "Application.h"
#include "Component.h"

class Resource
{
public:
	Resource() { m_uid = GenerateUUID(); }
	Resource(uint64_t UID) { this->m_uid = UID; }

	virtual ~Resource() { }

	//Number of references to this resource. Won't unload while it's over 0
	uint m_numReferences = 0;

	//Unique ID for this resource
	uint64_t m_uid = 0;

	//Filename this resource extraced the data from
	std::string m_name;

	virtual Component::Type GetType() = 0;

	template <typename T>
	T* Read() { return (T*)this; }
};

#endif // ! __R_RESOURCE__