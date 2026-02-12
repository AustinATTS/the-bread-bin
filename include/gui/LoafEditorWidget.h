#ifndef LOAFEDITORWIDGET_H
#define LOAFEDITORWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <memory>
#include "LoafEditor.h"

namespace BreadBin {
    namespace GUI {

        class LoafEditorWidget : public QWidget {
            Q_OBJECT

            public:
                explicit LoafEditorWidget(QWidget *parent = nullptr);
                ~LoafEditorWidget();

                void newLoaf();
                bool openLoaf(const QString &filepath);
                bool saveLoaf(const QString &filepath);
                bool saveLoaf(); // Save to current path
                bool hasUnsavedChanges() const;
                std::shared_ptr<Loaf> getCurrentLoaf() const;
                QString getCurrentFilePath() const;

            signals:
                void loafModified();
                void createNewScriptRequested();

            private slots:
                void onAddApplication();
                void onAddFile();
                void onAddScript();
                void onAddConfig();
                void onAddWebPage();
                void onRemoveItem();
                void onItemSelected();
                void onConfigureItem();
                void onNameChanged();
                void onDescriptionChanged();

            private:
                void setupUI();
                void updateLoafInfo();
                void refreshItemList();
                void connectSignals();
                QString generateItemId(const QString& name) const;

                std::shared_ptr<LoafEditor> editor_;

                QString currentFilePath_;

                QLineEdit *nameEdit_;
                QTextEdit *descriptionEdit_;
                QListWidget *itemList_;

                QPushButton *addAppButton_;
                QPushButton *addFileButton_;
                QPushButton *addScriptButton_;
                QPushButton *addConfigButton_;
                QPushButton *addWebPageButton_;
                QPushButton *removeButton_;
                QPushButton *configureButton_;

                QLabel *statusLabel_;
        };

    } // namespace GUI
} // namespace BreadBin

#endif // LOAFEDITORWIDGET_H
