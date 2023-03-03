#include "ParticleEditorModule.hpp"

bool ParticleEditorModule::IsDataDirty() const
{
	return m_isDataDirty;
}

void ParticleEditorModule::SetDataDirty(bool dirty)
{
	m_isDataDirty = dirty;
}
