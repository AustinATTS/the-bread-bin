#include "ThemeEditor.h"
#include <fstream>
#include <sstream>
#include <vector>

namespace {
    std::vector<std::string> ColourElementKeys() {
        return {"background", "foreground", "primary", "secondary", "accent"};
    }
}

namespace BreadBin {

    ThemeEditor::ThemeEditor ( ) : theme_name_("Default Theme"), unsaved_changes_(false) {
        colours_["background"] = Colour(255, 255, 255);
        colours_["foreground"] = Colour(0, 0, 0);
        colours_["primary"] = Colour(0, 120, 215);
        colours_["secondary"] = Colour(100, 100, 100);
        colours_["accent"] = Colour(0, 150, 136);

        fonts_["default"] = std::make_pair("Arial", 12);
        fonts_["heading"] = std::make_pair("Arial", 16);
        fonts_["code"] = std::make_pair("Courier New", 10);
    }

    ThemeEditor::~ThemeEditor ( ) {

    }

    void ThemeEditor::NewTheme (const std::string& name) {
        theme_name_ = name;
        unsaved_changes_ = true;
    }

    bool ThemeEditor::LoadTheme (const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        std::string first_line;
        std::getline(file, first_line);

        if (first_line.rfind("NAME:", 0) == 0) {
            theme_name_ = first_line.substr(5);

            std::string line;
            while (std::getline(file, line)) {
                if (line.rfind("PRIMARY_COLOUR:", 0) == 0) {
                    std::stringstream ss(line.substr(14));
                    int red, green, blue, alpha;
                    if (ss >> red >> green >> blue >> alpha) {
                        colours_["primary"] = Colour(red, green, blue, alpha);
                    }
                }
                else {
                    if (line.rfind("SECONDARY_COLOUR:", 0) == 0) {
                        std::stringstream ss(line.substr(16));
                        int red, green, blue, alpha;
                        if (ss >> red >> green >> blue >> alpha) {
                            colours_["secondary"] = Colour(red, green, blue, alpha);
                        }
                    }
                    else {
                        if (line.rfind("BACKGROUND_COLOUR:", 0) == 0) {
                            std::stringstream ss(line.substr(17));
                            int red, green, blue, alpha;
                            if (ss >> red >> green >> blue >> alpha) {
                                colours_["background"] = Colour(red, green, blue, alpha);
                            }
                        }
                        else {
                            if (line.rfind("TEXT_COLOUR:", 0) == 0) {
                                std::stringstream ss(line.substr(11));
                                int red, green, blue, alpha;
                                if (ss >> red >> green >> blue >> alpha) {
                                    colours_["foreground"] = Colour(red, green, blue, alpha);
                                }
                            }
                            else {
                                if (line.rfind("ACCENT_COLOUR:", 0) == 0) {
                                    std::stringstream ss(line.substr(13));
                                    int red, green, blue, alpha;
                                    if (ss >> red >> green >> blue >> alpha) {
                                        colours_["accent"] = Colour(red, green, blue, alpha);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else {
            theme_name_ = first_line;

            int colour_count = 0;
            file >> colour_count;
            file.ignore();
            for (int i = 0; i < colour_count; ++i) {
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
            file << "COLOUR_" << key << ":" << colour.red << " " << colour.green << " " << colour.blue << " " << colour.alpha << "\n";
        }

        for (const auto& font : fonts_) {
            file << "FONT_" << font.first << ":" << font.second.first << "," << font.second.second << "\n";
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
        return (it != colours_.end()) ? it->second : Colour(0, 0, 0);
    }

    void ThemeEditor::SetFont (const std::string& element, const std::string& font_name, int font_size) {
        fonts_[element] = std::make_pair(font_name, font_size);
        unsaved_changes_ = true;
    }

    std::string ThemeEditor::GetFont (const std::string& element) const {
        auto it = fonts_.find(element);
        return (it != fonts_.end()) ? it->second.first : "Arial";
    }

    int ThemeEditor::GetFontSize (const std::string& element) const {
        auto it = fonts_.find(element);
        return (it != fonts_.end()) ? it->second.second : 12;
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
