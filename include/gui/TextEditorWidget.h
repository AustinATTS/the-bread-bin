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
                explicit TextEditorWidget(QWidget *parent = nullptr);
                ~TextEditorWidget();

                bool openFile(const QString &filepath);
                bool saveFile(const QString &filepath);
                bool saveCurrentFile();
                bool hasUnsavedChanges() const;
                void newFile();

            signals:
                void fileModified();

            private slots:
                void onNewFile();
                void onOpenFile();
                void onSaveFile();
                void onFind();
                void onReplace();
                void onTextChanged();
                void onTabChanged(int index);
                void onCloseTab(int index);

            private:
                void setupUI();
                void createNewTab(const QString &title = "Untitled");
                void connectSignals();
                int getCurrentTabIndex() const;
                QPlainTextEdit* getCurrentEditor() const;

                std::vector<std::shared_ptr<TextEditor>> editors_;

                QTabWidget *tabWidget_;
                QPushButton *newFileButton_;
                QPushButton *openFileButton_;
                QPushButton *saveFileButton_;
                QPushButton *findButton_;
                QPushButton *replaceButton_;

                QLabel *statusLabel_;

                QStringList filePaths_;
        };

    } // namespace GUI
} // namespace BreadBin

#endif // TEXTEDITORWIDGET_H
