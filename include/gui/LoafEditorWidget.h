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
                explicit LoafEditorWidget (QWidget *parent = nullptr);
                ~LoafEditorWidget ( );

                void NewLoaf ( );
                bool OpenLoaf (const QString &filepath);
                bool SaveLoaf (const QString &filepath);
                bool SaveLoaf ( );
                bool HasUnsavedChanges ( ) const;
                std::shared_ptr<Loaf> GetCurrentLoaf ( ) const;
                QString GetCurrentFilePath ( ) const;

            signals:
                void LoafModified ( );
                void CreateNewScriptRequested ( );

            private slots:
                void OnAddApplication ( );
                void OnAddFile ( );
                void OnAddScript ( );
                void OnAddConfig ( );
                void OnAddWebPage ( );
                void OnRemoveItem ( );
                void OnItemSelected ( );
                void OnConfigureItem ( );
                void OnNameChanged ( );
                void OnDescriptionChanged ( );

            private:
                void SetupUI ( );
                void UpdateLoafInfo ( );
                void RefreshItemList ( );
                void ConnectSignals ( );
                QString GenerateItemId (const QString& name) const;

                std::shared_ptr<LoafEditor> editor_;

                QString current_file_path_;

                QLineEdit *name_edit_;
                QTextEdit *description_edit_;
                QListWidget *item_list_;

                QPushButton *add_app_button_;
                QPushButton *add_file_button_;
                QPushButton *add_script_button_;
                QPushButton *add_config_button_;
                QPushButton *add_web_page_button_;
                QPushButton *remove_button_;
                QPushButton *configure_button_;

                QLabel *status_label_;
        };

    } // namespace GUI
} // namespace BreadBin

#endif // LOAFEDITORWIDGET_H
