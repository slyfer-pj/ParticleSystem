#pragma once
#include <algorithm>
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Renderer/ParticleEmitterData.hpp"
#include "ThirdParty/ImGUI/imgui.h"
#include "ThirdParty/ImGUI/imgui_internal.h"

struct ImRect;
struct ImVec2;
struct ImDrawList;

constexpr float KEY_RADIUS = 5.f;
constexpr unsigned int MAX_KEYS = 8;
constexpr unsigned int MIN_KEYS = 2;
constexpr int NUM_CURVE_MODES = 2;
static const char* CURVE_MODES[NUM_CURVE_MODES] = { "Single", "RandomBetweenTwo" };

template <typename T>
class CurveEditor
{
public:
	CurveEditor(AnimatedCurve<T>* readKeys, bool canGoNegative = true);
	void UpdateWindow();
	bool IsWindowOpen() const { return m_isOpen; }

private:
	void DrawGridLines(const ImRect& windowRect, const ImVec2& size, ImDrawList* drawList) const;
	void DrawTextValues(const ImRect& windowRect, const ImVec2& size, ImDrawList* drawList) const;
	void UpdateAddKeyContextMenu(const ImRect& rect, const ImVec2& size);
	void UpdateDeleteKeyContextMenu(std::vector<AnimatedValueKey<T>>& keyArray, int keyIndex, const ImRect& rect, const ImVec2& size);
	void UpdateKeys(std::vector<AnimatedValueKey<T>>& keys, int curveNum, const ImRect& graphRect, const ImVec2& size, ImDrawList* drawList, ImU32 curveColor);
	ImVec2 GetKeyCoords(ImVec2 graphMins, ImVec2 graphDimensions, AnimatedValueKey<T>& key);
	void AddNewKey(std::vector<AnimatedValueKey<T>>& keyArray, const ImRect& rect);

private:
	//std::vector<AnimatedValueKey<T>>& m_keys;
	AnimatedCurve<T>* m_curve = nullptr;
	int m_indexOfLastHoveredButton = -1;
	int m_lastHoveredButtonCurveNum = 0;
	bool m_deleteLastHoveredKey = false;
	bool m_isOpen = true;
	bool m_canGoNegative = false;
	int m_curveMode = 0;
};

template <typename T>
void CurveEditor<T>::AddNewKey(std::vector<AnimatedValueKey<T>>& keyArray, const ImRect& rect)
{
	ImVec2 cursorPos = ImGui::GetCursorScreenPos();
	float time = RangeMap(cursorPos.x, rect.Min.x, rect.Max.x, 0.f, 1.f);
	float value = 1.f - RangeMap(cursorPos.y, rect.Min.y, rect.Max.y, 0.f, 1.f);
	AnimatedValueKey<T> newKey;
	newKey.SetValue(value);
	newKey.SetTime(time);
	keyArray.push_back(newKey);
	std::sort(keyArray.begin(), keyArray.end());
}

template <typename T>
CurveEditor<T>::CurveEditor(AnimatedCurve<T>* curve, bool canGoNegative /*= true*/)
	:m_curve(curve), m_canGoNegative(canGoNegative), m_curveMode(curve->IsToRandomBetweenCurves() ? 1 : 0)
{
}

template <typename T>
void CurveEditor<T>::UpdateKeys(std::vector<AnimatedValueKey<T>>& keys, int curveNum, const ImRect& graphRect, const ImVec2& size, ImDrawList* DrawList, ImU32 curveColor)
{
	const ImGuiIO& IO = ImGui::GetIO();
	for (int i = 0; i < keys.size(); i++)
	{
		//convert normalized key value and key time to imgui window coords.
		//ImVec2 keyCoords = ImVec2(graphRect.Min.x + (m_keys[i].GetTime() * size.x), graphRect.Min.y + ((1.f - m_keys[i].GetValue()) * size.y));		//need to invert y as (0, 0) is topleft in imgui
		ImVec2 keyCoords = GetKeyCoords(graphRect.Min, size, keys[i]);

		//here cursor is referred to the position where imgui will output. Set it where you current key is according to its window coords.
		ImGui::SetCursorScreenPos(ImVec2(keyCoords.x - KEY_RADIUS, keyCoords.y - KEY_RADIUS));
		ImGui::InvisibleButton(Stringf("key##%d%d", i, curveNum).c_str(), ImVec2(2.f * KEY_RADIUS, 2.f * KEY_RADIUS), ImGuiButtonFlags_MouseButtonLeft);

		//move the key if they your mouse cursor is on it.
		const bool is_active = ImGui::IsItemActive();
		if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			keyCoords.x += IO.MouseDelta.x;
			keyCoords.y += IO.MouseDelta.y;
			keyCoords.x = Clamp(keyCoords.x, graphRect.Min.x, graphRect.Max.x);
			keyCoords.y = Clamp(keyCoords.y, graphRect.Min.y, graphRect.Max.y);
		}
		/*if(i == 0)
			DebuggerPrintf(Stringf("Keycoord = %.2f, %.2f \n", keyCoords.x, keyCoords.y).c_str());*/

			//store last hovered item for deletion key context menu.
		if (ImGui::IsItemHovered())
		{
			m_indexOfLastHoveredButton = i;
			m_lastHoveredButtonCurveNum = curveNum;
		}

		//draw key
		DrawList->AddCircleFilled(ImVec2(keyCoords.x, keyCoords.y), KEY_RADIUS, curveColor, 16);

		//draw line to next key
		if (i < keys.size() - 1)
		{
			ImVec2 nextkeyCoords = GetKeyCoords(graphRect.Min, size, keys[i + 1]);
			//ImU32 lineColor = IM_COL32(255, 255, 255, 255);
			DrawList->AddLine(keyCoords, nextkeyCoords, curveColor, 3.f);
		}

		//convert back window coords to normalized values
		keys[i].SetTime((keyCoords.x - graphRect.Min.x) / size.x);
		float normalizedValue = 1.f - ((keyCoords.y - graphRect.Min.y) / size.y);
		if (m_canGoNegative)
			normalizedValue = (2.f * normalizedValue) - 1.f;
		keys[i].SetValue(normalizedValue);
	}
}

template <typename T>
ImVec2 CurveEditor<T>::GetKeyCoords(ImVec2 graphMins, ImVec2 graphDimensions, AnimatedValueKey<T>& key)
{
	ImVec2 keyCoords;
	keyCoords.x = graphMins.x + (key.GetTime() * graphDimensions.x);
	float worldValue = (1.f - (m_canGoNegative ? ((key.GetValue() + 1) / 2) : (key.GetValue()))) * graphDimensions.y;
	keyCoords.y = graphMins.y + worldValue;

	return keyCoords;
}

template <typename T>
void CurveEditor<T>::DrawTextValues(const ImRect& windowRect, const ImVec2& size, ImDrawList* drawList) const
{
	constexpr float TEXT_WIDTH = 20.f;	 //magic number
	const ImU32 textColor = IM_COL32(255, 255, 255, 255);
	//y axis values
	drawList->AddText(windowRect.Min, textColor, "1.0");
	drawList->AddText(ImVec2(windowRect.Min.x, windowRect.Min.y + size.y * 0.5f), textColor, m_canGoNegative ? "0.0" : "0.5");
	drawList->AddText(ImVec2(windowRect.Min.x, windowRect.Min.y + size.y), textColor, m_canGoNegative ? "-1.0" : "0.0");

	//x axis values
	//drawList->AddText(ImVec2(windowRect.Min.x, windowRect.Max.y), textColor, "1.0");
	drawList->AddText(ImVec2(windowRect.Min.x + TEXT_WIDTH + size.x * 0.5f, windowRect.Min.y + size.y), textColor, "0.5");
	drawList->AddText(ImVec2(windowRect.Min.x + TEXT_WIDTH + size.x, windowRect.Min.y + size.y), textColor, "1.0");
}

template <typename T>
void CurveEditor<T>::UpdateDeleteKeyContextMenu(std::vector<AnimatedValueKey<T>>& keyArray, int keyIndex, const ImRect& rect, const ImVec2& size)
{
	if (keyIndex < 0)
		return;

	ImVec2 keyCoords = GetKeyCoords(rect.Min, size, keyArray[keyIndex]);
	ImGui::SetCursorScreenPos(ImVec2(keyCoords.x - KEY_RADIUS, keyCoords.y - KEY_RADIUS));
	ImGui::InvisibleButton(Stringf("key##%d", m_indexOfLastHoveredButton).c_str(), ImVec2(2.f * KEY_RADIUS, 2.f * KEY_RADIUS), ImGuiButtonFlags_MouseButtonLeft);
	ImGui::OpenPopupOnItemClick("delete context menu", ImGuiPopupFlags_MouseButtonRight);
	if (ImGui::BeginPopup("delete context menu"))
	{
		if (ImGui::MenuItem("Delete Key", nullptr, false, keyArray.size() > MIN_KEYS))
		{
			m_deleteLastHoveredKey = true;
		}
		ImGui::EndPopup();
	}
}

template <typename T>
void CurveEditor<T>::UpdateAddKeyContextMenu(const ImRect& rect, const ImVec2& size)
{
	ImGui::InvisibleButton("context menu", size, ImGuiButtonFlags_AllowItemOverlap);
	ImGui::SetItemAllowOverlap();
	ImGui::OpenPopupOnItemClick("context menu", ImGuiPopupFlags_MouseButtonRight);
	if (ImGui::BeginPopup("context menu"))
	{
		if (m_curveMode == 0)
		{
			if (ImGui::MenuItem("Add Key", nullptr, false, m_curve->m_curveOneKeys.size() < MAX_KEYS))
			{
				AddNewKey(m_curve->m_curveOneKeys, rect);
			}
		}
		else
		{
			if (ImGui::MenuItem("Add Key (Red Curve)", nullptr, false, m_curve->m_curveOneKeys.size() < (MAX_KEYS / 2)))
			{
				AddNewKey(m_curve->m_curveOneKeys, rect);
			}
			if (ImGui::MenuItem("Add Key (Green Curve)", nullptr, false, m_curve->m_curveTwoKeys.size() < (MAX_KEYS / 2)))
			{
				AddNewKey(m_curve->m_curveTwoKeys, rect);
			}
		}
		ImGui::EndPopup();
	}
}

template <typename T>
void CurveEditor<T>::DrawGridLines(const ImRect& windowRect, const ImVec2& size, ImDrawList* drawList) const
{
	constexpr int verticalSubDivisions = 10;
	constexpr int horizontalSubDivisions = 4;

	float horizontalDistanceBetweenLines = size.x / (float)verticalSubDivisions;
	float verticalDistanceBetweenLines = size.y / (float)horizontalSubDivisions;
	ImU32 lineColor = IM_COL32(74, 74, 74, 255);
	float lineThickness = 0.5f;

	//vertical grid lines
	ImVec2 lineStart = ImVec2(windowRect.Min.x + horizontalDistanceBetweenLines, windowRect.Min.y);
	for (int i = 0; i < verticalSubDivisions - 1; i++)
	{
		drawList->AddLine(lineStart, ImVec2(lineStart.x, lineStart.y + size.y), lineColor, lineThickness);
		lineStart = ImVec2(lineStart.x + horizontalDistanceBetweenLines, lineStart.y);
	}

	//horizontal grid lines
	lineStart = ImVec2(windowRect.Min.x, windowRect.Min.y + verticalDistanceBetweenLines);
	for (int i = 0; i < horizontalSubDivisions - 1; i++)
	{
		drawList->AddLine(lineStart, ImVec2(lineStart.x + size.x, lineStart.y), lineColor, lineThickness);
		lineStart = ImVec2(lineStart.x, lineStart.y + verticalDistanceBetweenLines);
	}
}

template <typename T>
void CurveEditor<T>::UpdateWindow()
{
	ImGuiStyle& Style = ImGui::GetStyle();

	Style.WindowMinSize = ImVec2(400.f, 300.f);

	ImGui::Begin("Curve Editor", &m_isOpen);
	{
		const float padding[4] = { 30.f, 20.f, 50.f, 0.f };	//left, right, bottom, top
		ImGuiWindow* Window = ImGui::GetCurrentWindow();
		ImDrawList* DrawList = ImGui::GetWindowDrawList();
		ImRect graphRect = Window->ContentRegionRect;
		graphRect.Min.x += padding[0];
		graphRect.Max.x -= padding[1];
		graphRect.Min.y += padding[3];
		graphRect.Max.y -= padding[2];
		ImVec2 size = ImVec2(graphRect.Max.x - graphRect.Min.x, graphRect.Max.y - graphRect.Min.y);

		//draw window bg
		ImU32 borderColor = IM_COL32(255, 255, 255, 255);
		DrawList->AddRect(graphRect.Min, graphRect.Max, borderColor);
		ImU32 greyBg = IM_COL32(50, 50, 50, 255);
		DrawList->AddRectFilled(graphRect.Min, graphRect.Max, greyBg);
		DrawGridLines(graphRect, size, DrawList);
		DrawTextValues(Window->ContentRegionRect, size, DrawList);

		//update key context menu
		UpdateAddKeyContextMenu(graphRect, size);

		ImU32 curveOneColor = IM_COL32(255, 0, 0, 255);
		UpdateKeys(m_curve->m_curveOneKeys, 1, graphRect, size, DrawList, curveOneColor);
		if (m_curve->IsToRandomBetweenCurves())
		{
			if (m_curve->m_curveTwoKeys.size() < 2)
			{
				AnimatedValueKey<T> key;
				key.SetTime(0.f);
				key.SetValue(1.f);
				m_curve->AddKeyToCurveTwo(key);
				key.SetTime(1.f);
				m_curve->AddKeyToCurveTwo(key);
			}

			ImU32 curveTwoColor = IM_COL32(0, 255, 0, 255);
			UpdateKeys(m_curve->m_curveTwoKeys, 2, graphRect, size, DrawList, curveTwoColor);
		}

		std::vector<AnimatedValueKey<T>>& lastHoveredButtonKeyArray = m_lastHoveredButtonCurveNum == 1 ? m_curve->m_curveOneKeys : m_curve->m_curveTwoKeys;
		//update deletion key context menu
		UpdateDeleteKeyContextMenu(lastHoveredButtonKeyArray, m_indexOfLastHoveredButton, graphRect, size);
		
		ImGui::SetCursorScreenPos(ImVec2(graphRect.Min.x, graphRect.Min.y + size.y));
		ImGui::NewLine();
		ImGui::Combo("Curve Mode", &m_curveMode, CURVE_MODES, NUM_CURVE_MODES);
		if (m_curveMode == 0)
		{
			m_curve->SetToRandomBetweenCurves(false);
		}
		else
		{
			m_curve->SetToRandomBetweenCurves(true);
		}


		//delete a key if the user had pressed the delete key option in the context menu.
		if (m_deleteLastHoveredKey)
		{
			lastHoveredButtonKeyArray.erase(lastHoveredButtonKeyArray.begin() + m_indexOfLastHoveredButton);
			m_deleteLastHoveredKey = false;
		}
	}
	ImGui::End();

	
}
