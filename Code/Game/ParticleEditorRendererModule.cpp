#include "Engine/Renderer/Window.hpp"
#include "Game/ParticleEditorRendererModule.hpp"
#include "Game/Game.hpp"

const char* options[3] = { "Alpha", "Additive", "Opaque" };
const char* RENDER_MODE_OPTIONS[2] = { "Billboard", "HorizontalBillboard" };
const char* INITIAL_TEXTURE_DIRECTORY = "Data/Images";
extern Window* g_theWindow;

ParticleEditorRendererModule::ParticleEditorRendererModule(Game* game)
	:m_game(game)
{
}

void ParticleEditorRendererModule::LoadDataFromXML(const ParticleEmitterData& emitterData)
{
	m_renderMode = emitterData.m_renderMode;
	m_textureFilePath = emitterData.m_textureFilepath;
	m_isSpriteSheet = emitterData.m_isSpriteSheetTexture;
	m_spriteDimensions[0] = emitterData.m_spriteSheetGridLayout.x;
	m_spriteDimensions[1] = emitterData.m_spriteSheetGridLayout.y;
	m_blendMode = emitterData.m_blendMode;
	m_sortParticles = emitterData.m_sortParticles;
}

XmlElement* ParticleEditorRendererModule::SaveDataToXMLElement(tinyxml2::XMLDocument& doc)
{
	XmlElement* rendererElement = doc.NewElement("Renderer");
	rendererElement->SetAttribute("mode", RENDER_MODE_OPTIONS[static_cast<int>(m_renderMode)]);
	rendererElement->SetAttribute("texture", m_textureFilePath.c_str());
	rendererElement->SetAttribute("isSpriteSheet", m_isSpriteSheet);
	rendererElement->SetAttribute("dimensions", IntVec2(m_spriteDimensions[0], m_spriteDimensions[1]).ToXMLString().c_str());
	rendererElement->SetAttribute("blend", options[static_cast<int>(m_blendMode)]);
	rendererElement->SetAttribute("sortParticles", m_sortParticles);
	return rendererElement;
}

void ParticleEditorRendererModule::UpdateWindow()
{
	if (ImGui::CollapsingHeader("Renderer"))
	{
		ImGui::Combo("Render Mode", (int*)&m_renderMode, RENDER_MODE_OPTIONS, 2);
		ImGui::Text("Texture - \"%s\"", m_textureFilePath.c_str());
		if (ImGui::Button("Select Texture"))
		{
			m_textureFilePath = g_theWindow->GetFileNameFromFileExploreDialogueBox(INITIAL_TEXTURE_DIRECTORY);
			if (m_textureFilePath == "")
				m_textureFilePath = "Default";
		}
		ImGui::SameLine();
		if (ImGui::Button("Clear Texture"))
		{
			m_textureFilePath = "Default";
		}
		ImGui::Checkbox("Is Sprite Sheet", &m_isSpriteSheet);
		if(m_isSpriteSheet)
		{
			ImGui::InputInt2("SpriteDimensions", m_spriteDimensions);
		}

		static int selectedOption = static_cast<int>(m_blendMode);
		if (ImGui::BeginCombo("Blend Mode", options[selectedOption]))
		{
			for (int n = 0; n < 3; n++)
			{
				const bool is_selected = (selectedOption == n);
				if (ImGui::Selectable(options[n], is_selected))
					selectedOption = n;

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		m_blendMode = static_cast<BlendMode>(selectedOption);

		ImGui::Checkbox("Sort Particles", &m_sortParticles);
	}
}
