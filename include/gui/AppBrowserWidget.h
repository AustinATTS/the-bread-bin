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

namespace BreadBin {
    namespace GUI {

        class AppBrowserWidget : public QWidget {
            Q_OBJECT

            public:
                explicit AppBrowserWidget(QWidget *parent = nullptr);
                ~AppBrowserWidget();

                void refreshApplications();

                const AppInfo* getSelectedApplication() const;

            signals:
                void applicationSelected(const AppInfo& appInfo);
                void addApplicationRequested(const AppInfo& appInfo);

            private slots:
                void onScanClicked();
                void onSearchChanged(const QString& text);
                void onCategoryChanged(const QString& category);
                void onApplicationSelected();
                void onAddToLoafClicked();

            private:
                void setupUI();
                void connectSignals();
                void updateApplicationList();
                void updateApplicationDetails(const AppInfo& appInfo);
                void populateCategories();

                std::shared_ptr<AppDiscovery> discovery_;
                std::vector<AppInfo> filteredApps_;

                QLineEdit *searchEdit_;
                QComboBox *categoryCombo_;
                QListWidget *appList_;
                QTextEdit *detailsText_;
                QPushButton *scanButton_;
                QPushButton *addButton_;
                QLabel *statusLabel_;
        };

    } // namespace GUI
} // namespace BreadBin

#endif // APPBROWSERWIDGET_H
