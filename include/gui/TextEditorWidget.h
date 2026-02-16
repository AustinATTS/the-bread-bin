#ifndef TEXTEDITORWIDGET_H
#define TEXTEDITORWIDGET_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QShowEvent>
#include <memory>
#include "TextEditor.h"

namespace BreadBin {
    namespace GUI {

        class TextEditorWidget : public QWidget {
            Q_OBJECT

            public:
                explicit TextEditorWidget (QWidget *parent = nullptr);
                ~TextEditorWidget ( ) override;

                enum class DocumentType {
                    PlainText,
                    Script,
                    Theme,
                    Loaf,
                    Json,
                    Yaml,
                    Xml,
                    Ini
                };

                bool OpenFile (const QString &filepath);
                bool SaveFile (const QString &filepath);
                bool SaveCurrentFile ( );
                bool HasUnsavedChanges ( ) const;
                void NewFile ( );
                void NewScriptFile (const QString& loaf_name = "");

            signals:
                void FileModified ( );

            protected:
                void showEvent (QShowEvent* event) override;

            private slots:
                void OnNewFile ( );
                void OnOpenFile ( );
                void OnSaveFile ( );
                void OnRunScript ( );
                void OnFind ( );
                void OnReplace ( );
                void OnTextChanged ( );
                void OnTabChanged (int index);
                void OnCloseTab (int index);

            private:
                enum class FirstOpenAction {
                    OpenExisting,
                    CreateNew,
                    Cancel
                };

                void SetupUI ( );
                void CreateNewTab (const QString &title = "Untitled", DocumentType type = DocumentType::PlainText);
                void ConnectSignals ( );
                int GetCurrentTabIndex ( ) const;
                QPlainTextEdit* GetCurrentEditor ( ) const;
                DocumentType DetectDocumentType (const QString& filepath, const QString& content) const;
                QString SuggestedDirectoryForType (DocumentType type) const;
                QString SuggestedExtensionForType (DocumentType type) const;
                QString SuggestedFilterForType (DocumentType type) const;
                DocumentType PromptForDocumentType (bool* accepted, bool scripts_only = false) const;
                QString PromptForScriptExtension (bool* accepted) const;
                FirstOpenAction PromptForFirstOpenAction ( ) const;
                void ApplySyntaxHighlighting (int index, DocumentType type);
                void UpdateRunScriptButtonState (int index);

                std::vector<std::shared_ptr<TextEditor>> editors_;
                std::vector<DocumentType> document_types_;

                QTabWidget *tab_widget_;
                QPushButton *new_file_button_;
                QPushButton *open_file_button_;
                QPushButton *save_file_button_;
                QPushButton *run_script_button_;
                QPushButton *find_button_;
                QPushButton *replace_button_;

                QLabel *status_label_;

                QStringList file_paths_;
                QStringList preferred_extensions_;
                QStringList script_loaf_names_;
                bool prompted_on_first_show_;
        };

    } // namespace GUI
} // namespace BreadBin

#endif // TEXTEDITORWIDGET_H