#ifndef THEME_EDITOR_H
#define THEME_EDITOR_H

#include <string>
#include <map>

namespace BreadBin {

    class ThemeEditor {
        public:
            struct Colour {
                int r, g, b, a;
                Colour(int red = 0, int green = 0, int blue = 0, int alpha = 255)
                    : r(red), g(green), b(blue), a(alpha) {}
            };

            ThemeEditor();
            ~ThemeEditor();

            void newTheme(const std::string& name);
            bool loadTheme(const std::string& filepath);
            bool saveTheme(const std::string& filepath);

            void setColour(const std::string& element, const Colour& colour);
            Colour getColour(const std::string& element) const;

            void setFont(const std::string& element, const std::string& fontName, int fontSize);
            std::string getFont(const std::string& element) const;
            int getFontSize(const std::string& element) const;

            void setStyle(const std::string& property, const std::string& value);
            std::string getStyle(const std::string& property) const;

            void setThemeName(const std::string& name);
            std::string getThemeName() const;
            bool hasUnsavedChanges() const;

        private:
            std::string themeName_;
            std::map<std::string, Colour> colours_;
            std::map<std::string, std::pair<std::string, int>> fonts_;
            std::map<std::string, std::string> styles_;
            bool unsavedChanges_;
    };

} // namespace BreadBin

#endif // THEME_EDITOR_H
