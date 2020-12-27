#pragma once

using namespace tinyxml2;

class XMLHelper sealed {
  public:
    static XMLElement* FirstElement(XMLElement* element, std::string elementName);

    static XMLElement* FindByAttribute(XMLElement* element, std::string attribute, std::string value);

    static XMLElement* FindByAttribute(XMLElement* element, std::string name, std::string attribute, std::string value);

    template <typename Predicate>
    static void ForEach(XMLElement* element, std::string name, Predicate&& pred) {
        const char * cstr = name.empty() ? nullptr : name.c_str();
        for (auto e = element->FirstChildElement(cstr); e != nullptr; e = e->NextSiblingElement(cstr)) {
            if (e) {
                pred(e);
            }
        }
    }

    template <typename Predicate>
    static XMLElement* FindIf(XMLElement* element, std::string name, Predicate&& pred) {
        const char * cstr = name.empty() ? nullptr : name.c_str();
        for (auto e = element->FirstChildElement(cstr); e != nullptr; e = e->NextSiblingElement(cstr)) {
            if (e && pred(e)) {
                return e;
            }
        }

        return nullptr;
    }
};
