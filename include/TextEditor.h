#ifndef TEXT_EDITOR_H
#define TEXT_EDITOR_H

#include <string>
#include <vector>

namespace BreadBin {

    class TextEditor {
        public:
            TextEditor ( );
            ~TextEditor ( );

            bool OpenFile (const std::string& filepath);
            bool SaveFile (const std::string& filepath);
            bool CloseFile ( );

            void SetContent (const std::string& content);
            std::string GetContent ( ) const;
            void InsertText (int line, int column, const std::string& text);
            void DeleteText (int start_line, int start_column, int end_line, int end_column);
            void ReplaceText (int start_line, int start_column, int end_line, int end_column, const std::string& text);

            int GetLineCount ( ) const;
            std::string GetLine (int line_number) const;

            bool Find (const std::string& search_text, int& line, int& column);
            int ReplaceAll (const std::string& search_text, const std::string& replace_text);

            bool HasUnsavedChanges ( ) const;
            std::string GetCurrentFilePath ( ) const;

        private:
            std::vector<std::string> lines_;
            std::string current_file_path_;
            bool unsaved_changes_;
    };

} // namespace BreadBin

#endif // TEXT_EDITOR_H