#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QString>
#include "AppDiscovery.h"

namespace BreadBin {
    namespace GUI {

        class HomeWidget;
        class LoafEditorWidget;
        class TextEditorWidget;
        class ThemeEditorWidget;
        class LoafRuntimeWidget;
        class AppBrowserWidget;
        class LoafBrowserWidget;
        class ThemeBrowserWidget;

        class MainWindow : public QMainWindow {
            Q_OBJECT

            public:
                explicit MainWindow(QWidget *parent = nullptr);
                ~MainWindow();

            protected:
                void closeEvent(QCloseEvent *event) override;

            private slots:
                void newLoaf();
                void openLoaf();
                bool openLoafFile(const QString& filepath);
                void saveLoaf();
                void saveLoafAs();
                void showAbout();
                void applyTheme();
                void onAddApplicationFromBrowser(const AppInfo& appInfo);
                void onOpenLoafFromBrowser(const QString& filepath);
                void onApplyThemeFromBrowser(const QString& filepath);
                void onOpenLoafInTextEditor(const QString& filepath);

            private:
                void createMenus();
                void createToolBar();
                void createCentralWidget();
                void applyWarmTheme();
                void applyThemeFromFile(const QString& filepath, bool persistSelection = true);
                QString getThemeSettingsFile() const;
                QString getDefaultLoafDirectory() const;
                bool checkUnsavedChanges();

                QTabWidget *tabWidget_;

                HomeWidget *homeWidget_;
                LoafEditorWidget *loafEditor_;
                TextEditorWidget *textEditor_;
                ThemeEditorWidget *themeEditor_;
                LoafRuntimeWidget *runtimeWidget_;
                AppBrowserWidget *appBrowser_;
                LoafBrowserWidget *loafBrowser_;
                ThemeBrowserWidget *themeBrowser_;

                QMenu *fileMenu_;
                QMenu *editMenu_;
                QMenu *viewMenu_;
                QMenu *helpMenu_;

                QToolBar *mainToolBar_;
                QString currentThemePath_;
        };

    } // namespace GUI
} // namespace BreadBin

#endif // MAINWINDOW_H
