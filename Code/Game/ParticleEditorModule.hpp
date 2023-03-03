#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/AnimatedValue.hpp"

struct ParticleEmitterData;

class ParticleEditorModule
{
public:
	virtual ~ParticleEditorModule() = default;

	virtual void LoadDataFromXML(const ParticleEmitterData& emitterData) = 0;
	virtual XmlElement* SaveDataToXMLElement(tinyxml2::XMLDocument& doc) = 0;
	virtual void UpdateWindow() = 0;
	bool IsDataDirty() const;
	void SetDataDirty(bool dirty);

protected:
	template <typename T>
	void WriteKeysToXMLElement(tinyxml2::XMLDocument& doc, XmlElement* parent, AnimatedCurve<T>& curve);

protected:
	bool m_isDataDirty = false;
};

template <typename T>
void ParticleEditorModule::WriteKeysToXMLElement(tinyxml2::XMLDocument& doc, XmlElement* parent, AnimatedCurve<T>& curve)
{
	XmlElement* curveOneElement = curve.IsToRandomBetweenCurves() ? doc.NewElement("CurveOne") : nullptr;
	if (curveOneElement)
		parent->InsertEndChild(curveOneElement);
	for (int i = 0; i < curve.m_curveOneKeys.size(); i++)
	{
		XmlElement* keyElement = doc.NewElement("key");
		keyElement->SetAttribute("value", curve.m_curveOneKeys[i].GetValue());
		keyElement->SetAttribute("time", curve.m_curveOneKeys[i].GetTime());
		curveOneElement ? curveOneElement->InsertEndChild(keyElement) : parent->InsertEndChild(keyElement);
	}
	if (curve.IsToRandomBetweenCurves())
	{
		XmlElement* curveTwoElement = doc.NewElement("CurveTwo");
		parent->InsertEndChild(curveTwoElement);
		for (int i = 0; i < curve.m_curveTwoKeys.size(); i++)
		{
			XmlElement* keyElement = doc.NewElement("key");
			keyElement->SetAttribute("value", curve.m_curveTwoKeys[i].GetValue());
			keyElement->SetAttribute("time", curve.m_curveTwoKeys[i].GetTime());
			curveTwoElement->InsertEndChild(keyElement);
		}
	}
}
