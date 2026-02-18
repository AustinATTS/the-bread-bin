#ifndef THEME_EDITOR_H
#define THEME_EDITOR_H

#include <string>
#include <map>

namespace BreadBin {
    class ThemeEditor {
        public:
            struct Colour {
                int red, green, blue, alpha;
                explicit Colour (int red = 0, int green = 0, int blue = 0, int alpha = 255) : red(red), green(green), blue(blue), alpha(alpha) {}
            };

            ThemeEditor ( );
            ~ThemeEditor ( );

            void NewTheme (const std::string& name);
            bool LoadTheme (const std::string& filepath);
            bool SaveTheme (const std::string& filepath);
            void SetColour (const std::string& element, const Colour& colour);
            [[nodiscard]] Colour GetColour (const std::string& element) const;
            void SetFont (const std::string& element, const std::string& font_name, int font_size);
            [[nodiscard]] std::string GetFont (const std::string& element) const;
            [[nodiscard]] int GetFontSize (const std::string& element) const;
            void SetStyle (const std::string& property, const std::string& value);
            [[nodiscard]] std::string GetStyle (const std::string& property) const;
            void SetThemeName (const std::string& name);
            [[nodiscard]] std::string GetThemeName ( ) const;
            [[nodiscard]] bool HasUnsavedChanges ( ) const;

        private:
            std::string theme_name_;
            std::map<std::string, Colour> colours_;
            std::map<std::string, std::pair<std::string, int>> fonts_;
            std::map<std::string, std::string> styles_;
            bool unsaved_changes_;
    };
} // namespace BreadBin

#endif // THEME_EDITOR_H