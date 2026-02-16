#include "TextEditor.h"
#include <fstream>
#include <sstream>

namespace BreadBin {

    TextEditor::TextEditor ( ) : current_file_path_(""), unsaved_changes_(false) {

    }

    TextEditor::~TextEditor ( ) {
        CloseFile();
    }

    bool TextEditor::OpenFile (const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        lines_.clear();
        std::string line;
        while (std::getline(file, line)) {
            lines_.push_back(line);
        }

        current_file_path_ = filepath;
        unsaved_changes_ = false;
        file.close();
        return true;
    }

    bool TextEditor::SaveFile (const std::string& filepath) {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        for (const auto& line : lines_) {
            file << line << "\n";
        }

        current_file_path_ = filepath;
        unsaved_changes_ = false;
        file.close();
        return true;
    }

    bool TextEditor::CloseFile ( ) {
        lines_.clear();
        current_file_path_.clear();
        unsaved_changes_ = false;
        return true;
    }

    void TextEditor::SetContent (const std::string& content) {
        lines_.clear();
        std::istringstream iss(content);
        std::string line;
        while (std::getline(iss, line)) {
            lines_.push_back(line);
        }
        unsaved_changes_ = true;
    }

    std::string TextEditor::GetContent ( ) const {
        std::ostringstream oss;
        for (size_t i = 0; i < lines_.size(); ++i) {
            oss << lines_[i];
            if (i < lines_.size() - 1) {
                oss << "\n";
            }
        }
        return oss.str();
    }

    void TextEditor::InsertText (int line, int column, const std::string& text) {
        if (line < 0 || line >= static_cast<int>(lines_.size())) {
            return;
        }

        if (column < 0 || column > static_cast<int>(lines_[line].length())) {
            return;
        }

        lines_[line].insert(column, text);
        unsaved_changes_ = true;
    }

    void TextEditor::DeleteText (int start_line, int start_column, int end_line, int end_column) {
        if (start_line < 0 || start_line >= static_cast<int>(lines_.size()) ||
            end_line < 0 || end_line >= static_cast<int>(lines_.size())) {
            return;
        }

        if (start_line == end_line) {
            if (start_column < 0 || end_column > static_cast<int>(lines_[start_line].length())) {
                return;
            }
            lines_[start_line].erase(start_column, end_column - start_column);
        }
        unsaved_changes_ = true;
    }

    void TextEditor::ReplaceText (int start_line, int start_column, int end_line, int end_column, const std::string& text) {
        DeleteText(start_line, start_column, end_line, end_column);
        InsertText(start_line, start_column, text);
    }

    int TextEditor::GetLineCount ( ) const {
        return static_cast<int>(lines_.size());
    }

    std::string TextEditor::GetLine (int line_number) const {
        if (line_number < 0 || line_number >= static_cast<int>(lines_.size())) {
            return "";
        }
        return lines_[line_number];
    }

    bool TextEditor::Find (const std::string& search_text, int& line, int& column) {
        for (size_t i = 0; i < lines_.size(); ++i) {
            size_t pos = lines_[i].find(search_text);
            if (pos != std::string::npos) {
                line = static_cast<int>(i);
                column = static_cast<int>(pos);
                return true;
            }
        }
        return false;
    }

    int TextEditor::ReplaceAll (const std::string& search_text, const std::string& replace_text) {
        int count = 0;
        for (auto& line : lines_) {
            size_t pos = 0;
            while ((pos = line.find(search_text, pos)) != std::string::npos) {
                line.replace(pos, search_text.length(), replace_text);
                pos += replace_text.length();
                count++;
            }
        }
        if (count > 0) {
            unsaved_changes_ = true;
        }
        return count;
    }

    bool TextEditor::HasUnsavedChanges ( ) const {
        return unsaved_changes_;
    }

    std::string TextEditor::GetCurrentFilePath ( ) const {
        return current_file_path_;
    }

} // namespace BreadBin