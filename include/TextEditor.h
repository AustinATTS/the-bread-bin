#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#include <string>
#include <vector>

namespace BreadBin {

    class TextEditor {
        public:
            TextEditor();
            ~TextEditor();

            bool openFile(const std::string& filepath);
            bool saveFile(const std::string& filepath);
            bool closeFile();

            void setContent(const std::string& content);
            std::string getContent() const;
            void insertText(int line, int column, const std::string& text);
            void deleteText(int startLine, int startColumn, int endLine, int endColumn);
            void replaceText(int startLine, int startColumn, int endLine, int endColumn, const std::string& text);

            int getLineCount() const;
            std::string getLine(int lineNumber) const;

            bool find(const std::string& searchText, int& line, int& column);
            int replaceAll(const std::string& searchText, const std::string& replaceText);

            bool hasUnsavedChanges() const;
            std::string getCurrentFilePath() const;

        private:
            std::vector<std::string> lines_;
            std::string currentFilePath_;
            bool unsavedChanges_;
    };

} // namespace BreadBin

#endif // TEXT_EDITOR_H
