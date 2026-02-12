#ifndef TEXTEDITORWIDGET_H
#define TEXTEDITORWIDGET_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <memory>
#include "TextEditor.h"

namespace BreadBin {
    namespace GUI {

        class TextEditorWidget : public QWidget {
            Q_OBJECT

            public:
                explicit TextEditorWidget (QWidget *parent = nullptr);
                ~TextEditorWidget ( );

                bool OpenFile (const QString &filepath);
                bool SaveFile (const QString &filepath);
                bool SaveCurrentFile ( );
                bool HasUnsavedChanges ( ) const;
                void NewFile ( );

            signals:
                void FileModified ( );

            private slots:
                void OnNewFile ( );
                void OnOpenFile ( );
                void OnSaveFile ( );
                void OnFind ( );
                void OnReplace ( );
                void OnTextChanged ( );
                void OnTabChanged (int index);
                void OnCloseTab (int index);

            private:
                void SetupUI ( );
                void CreateNewTab (const QString &title = "Untitled");
                void ConnectSignals ( );
                int GetCurrentTabIndex ( ) const;
                QPlainTextEdit* GetCurrentEditor ( ) const;

                std::vector<std::shared_ptr<TextEditor>> editors_;

                QTabWidget *tab_widget_;
                QPushButton *new_file_button_;
                QPushButton *open_file_button_;
                QPushButton *save_file_button_;
                QPushButton *find_button_;
                QPushButton *replace_button_;

                QLabel *status_label_;

                QStringList file_paths_;
        };

    } // namespace GUI
} // namespace BreadBin

#endif // TEXTEDITORWIDGET_H
