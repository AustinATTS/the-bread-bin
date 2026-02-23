#ifndef APPBROWSERWIDGET_H
#define APPBROWSERWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <memory>
#include "AppDiscovery.h"

namespace BreadBin::GUI {
    class AppBrowserWidget : public QWidget {
        Q_OBJECT

        public:
            explicit AppBrowserWidget (QWidget *parent = nullptr);
            ~AppBrowserWidget ( ) override;

            void RefreshApplications ( );
            [[nodiscard]] const AppInfo* GetSelectedApplication ( ) const;

        signals:
            void ApplicationSelected (const AppInfo& app_info);
            void AddApplicationRequested (const AppInfo& app_info);

        private slots:
            void OnScanClicked ( );
            void OnSearchChanged (const QString& text);
            void OnCategoryChanged (const QString& category);
            void OnApplicationSelected ( );
            void OnAddToLoafClicked ( );

        private:
            void SetupUI ( );
            void ConnectSignals ( );
            void UpdateApplicationList ( );
            void UpdateApplicationDetails (const AppInfo& app_info);
            void PopulateCategories ( ) const;

            std::shared_ptr<AppDiscovery> discovery_;
            std::vector<AppInfo> filtered_apps_;
            QLineEdit *search_edit_;
            QComboBox *category_combo_;
            QListWidget *app_list_;
            QTextEdit *details_text_;
            QPushButton *scan_button_;
            QPushButton *add_button_;
            QLabel *status_label_;
    };
} // namespace BreadBin::GUI

#endif // APPBROWSERWIDGET_H