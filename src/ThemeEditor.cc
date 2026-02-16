#include "ThemeEditor.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

namespace BreadBin {
    namespace {
        constexpr int k_default_alpha = 255;

        std::vector<std::string> ColourElementKeys ( ) {
            return {"background", "foreground", "primary", "secondary", "accent", "default", "heading", "code"};
        }

        std::string Trim (const std::string& value) {
            const auto first = value.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) {
                return "";
            }
            const auto last = value.find_last_not_of(" \t\r\n");
            return value.substr(first, (last - first + 1));
        }

        ThemeEditor::Colour ParseColourValue (const std::string& value, const ThemeEditor::Colour& fallback) {
            std::stringstream colour_stream(value);
            int red = 0;
            int green = 0;
            int blue = 0;
            int alpha = k_default_alpha;

            if (!(colour_stream >> red >> green >> blue)) {
                return fallback;
            }

            if (!(colour_stream >> alpha)) {
                alpha = k_default_alpha;
            }

            return ThemeEditor::Colour(red, green, blue, alpha);
        }

        void ResetToDefaults (std::map<std::string, ThemeEditor::Colour>* colours, std::map<std::string, std::pair<std::string, int>>* fonts, std::map<std::string, std::string>* styles) {
            colours->clear();
            (*colours)["background"] = ThemeEditor::Colour(250, 247, 242);
            (*colours)["foreground"] = ThemeEditor::Colour(45, 31, 15);
            (*colours)["primary"] = ThemeEditor::Colour(212, 165, 116);
            (*colours)["secondary"] = ThemeEditor::Colour(245, 240, 232);
            (*colours)["accent"] = ThemeEditor::Colour(194, 152, 96);
            (*colours)["default"] = ThemeEditor::Colour(45, 31, 15);
            (*colours)["heading"] = ThemeEditor::Colour(93, 78, 55);
            (*colours)["code"] = ThemeEditor::Colour(245, 240, 232);

            fonts->clear();
            (*fonts)["default"] = std::make_pair("Sans Serif", 13);
            (*fonts)["global"] = std::make_pair("Sans Serif", 13);
            (*fonts)["heading"] = std::make_pair("Sans Serif", 14);
            (*fonts)["code"] = std::make_pair("Monospace", 12);

            styles->clear();
            (*styles)["USE_ELEMENT_FONTS"] = "false";
        }

        bool ParseThemeEntry (const std::string& line, std::string& key, std::string& value) {
            if (line.empty()) {
                return false;
            }

            const auto seperator = line.find_first_of(":=");
            if (seperator == std::string::npos) {
                return false;
            }

            key = Trim(line.substr(0, seperator));
            value = Trim(line.substr(seperator + 1));
            return !key.empty();
        }
    }

    ThemeEditor::ThemeEditor ( ) : theme_name_("Default Theme"), unsaved_changes_(false) {
        ResetToDefaults(&colours_, &fonts_, &styles_);
    }

    ThemeEditor::~ThemeEditor ( ) {

    }

    void ThemeEditor::NewTheme (const std::string& name) {
        theme_name_ = name;
        ResetToDefaults(&colours_, &fonts_, &styles_);
        unsaved_changes_ = true;
    }

    bool ThemeEditor::LoadTheme (const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        std::string first_line;
        std::getline(file, first_line);
        std::string key;
        std::string value;

        if (ParseThemeEntry(first_line, key, value) && key == "NAME") {
            theme_name_ = value;

            std::string line;
            while (std::getline(file, line)) {
                if (!ParseThemeEntry(line, key, value)) {
                    continue;
                }

                if (key == "PRIMARY_COLOUR") {
                    colours_["primary"] = ParseColourValue(value, colours_["primary"]);
                    continue;
                }

                if (key == "SECONDARY_COLOUR") {
                    colours_["secondary"] = ParseColourValue(value, colours_["secondary"]);
                    continue;
                }

                if (key == "BACKGROUND_COLOUR") {
                    colours_["background"] = ParseColourValue(value, colours_["background"]);
                    continue;
                }

                if (key == "TEXT_COLOUR" || key == "TEXT_COLOR") {
                    colours_["foreground"] = ParseColourValue(value, colours_["foreground"]);
                    continue;
                }

                if (key == "ACCENT_COLOUR") {
                    colours_["accent"] = ParseColourValue(value, colours_["accent"]);
                    continue;
                }

                if (key.rfind("COLOUR_", 0) == 0) {
                    const std::string element_key = key.substr(7);
                    if (!element_key.empty()) {
                        colours_[element_key] = ParseColourValue(value, Colour());
                    }
                    continue;
                }

                if (key.rfind("FONT_", 0) == 0) {
                    const auto comma_position = value.rfind(',');
                    if (comma_position != std::string::npos) {
                        const std::string font_name = Trim(value.substr(0, comma_position));
                        int parsed_size = 13;
                        std::stringstream size_parser(Trim(value.substr(comma_position + 1)));
                        if (size_parser >> parsed_size) {
                            parsed_size = std::max(6, parsed_size);
                        }
                        if (!font_name.empty()) {
                            fonts_[key.substr(5)] = std::make_pair(font_name, parsed_size);
                        }
                    }
                    continue;
                }

                styles_[key] = value;
            }
        }
        else {
            theme_name_ = first_line;

            int color_count = 0;
            file >> color_count;
            file.ignore();
            for (int i = 0; i < color_count; ++i) {
                std::string element;
                int red, green, blue, alpha;
                std::getline(file, element);
                file >> red >> green >> blue >> alpha;
                file.ignore();
                colours_[element] = Colour(red, green, blue, alpha);
            }

            int font_count = 0;
            file >> font_count;
            file.ignore();
            for (int i = 0; i < font_count; ++i) {
                std::string element, font_name;
                int font_size;
                std::getline(file, element);
                std::getline(file, font_name);
                file >> font_size;
                file.ignore();
                fonts_[element] = std::make_pair(font_name, font_size);
            }
        }

        file.close();
        unsaved_changes_ = false;
        return true;
    }

    bool ThemeEditor::SaveTheme (const std::string& filepath) {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        file << "NAME:" << theme_name_ << "\n";

        const Colour background = GetColour("background");
        const Colour foreground = GetColour("foreground");
        const Colour primary = GetColour("primary");
        const Colour secondary = GetColour("secondary");
        const Colour accent = GetColour("accent");

        file << "PRIMARY_COLOUR:" << primary.red << " " << primary.green << " " << primary.blue << " " << primary.alpha << "\n";
        file << "SECONDARY_COLOUR:" << secondary.red << " " << secondary.green << " " << secondary.blue << " " << secondary.alpha << "\n";
        file << "BACKGROUND_COLOUR:" << background.red << " " << background.green << " " << background.blue << " " << background.alpha << "\n";
        file << "TEXT_COLOUR:" << foreground.red << " " << foreground.green << " " << foreground.blue << " " << foreground.alpha << "\n";
        file << "ACCENT_COLOUR:" << accent.red << " " << accent.green << " " << accent.blue << " " << accent.alpha << "\n";

        for (const auto& key : ColourElementKeys()) {
            const Colour colour = GetColour(key);
            file << "COLOUR_" << key << "=" << colour.red << " " << colour.green << " " << colour.blue << " " << colour.alpha << "\n";
        }

        for (const auto& font : fonts_) {
            file << "FONT_" << font.first << "=" << font.second.first << "," << font.second.second << "\n";
        }

        for (const auto& style : styles_) {
            file << style.first << "=" << style.second << "\n";
        }

        file.close();
        unsaved_changes_ = false;
        return true;
    }

    void ThemeEditor::SetColour (const std::string& element, const Colour& colour) {
        colours_[element] = colour;
        unsaved_changes_ = true;
    }

    ThemeEditor::Colour ThemeEditor::GetColour (const std::string& element) const {
        auto it = colours_.find(element);
        if (it != colours_.end()) {
            return it->second;
        }

        if (element == "default" || element == "heading" || element == "code") {
            auto foreground = colours_.find("foreground");
            if (foreground != colours_.end()) {
                return foreground->second;
            }
        }

        if (element == "secondary") {
            auto background = colours_.find("background");
            if (background != colours_.end()) {
                return background->second;
            }
        }

        return Colour(45,31,15);
    }

    void ThemeEditor::SetFont (const std::string& element, const std::string& font_name, int font_size) {
        fonts_[element] = std::make_pair(font_name, font_size);
        unsaved_changes_ = true;
    }

    std::string ThemeEditor::GetFont (const std::string& element) const {
        auto it = fonts_.find(element);
        return (it != fonts_.end()) ? it->second.first : "Sans Serif";
    }

    int ThemeEditor::GetFontSize (const std::string& element) const {
        auto it = fonts_.find(element);
        return (it != fonts_.end()) ? it->second.second : 13;
    }

    void ThemeEditor::SetStyle (const std::string& property, const std::string& value) {
        styles_[property] = value;
        unsaved_changes_ = true;
    }

    std::string ThemeEditor::GetStyle (const std::string& property) const {
        auto it = styles_.find(property);
        return (it != styles_.end()) ? it->second : "";
    }

    void ThemeEditor::SetThemeName (const std::string& name) {
        theme_name_ = name;
        unsaved_changes_ = true;
    }

    std::string ThemeEditor::GetThemeName ( ) const {
        return theme_name_;
    }

    bool ThemeEditor::HasUnsavedChanges ( ) const {
        return unsaved_changes_;
    }

} // namespace BreadBin