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
                explicit MainWindow (QWidget *parent = nullptr);
                ~MainWindow ( );

            protected:
                void closeEvent (QCloseEvent *event) override;

            private slots:
                void NewLoaf ( );
                void OpenLoaf ( );
                bool OpenLoafFile (const QString& filepath);
                void SaveLoaf ( );
                void SaveLoafAs ( );
                void SaveCurrentContext ( );
                void ShowAbout ( );
                void ApplyTheme ( );
                void OnAddApplicationFromBrowser (const AppInfo& app_info);
                void OnOpenLoafFromBrowser (const QString& filepath);
                void OnApplyThemeFromBrowser (const QString& filepath);
                void OnOpenLoafInTextEditor (const QString& filepath);

            private:
                void CreateMenus ( );
                void CreateToolBar ( );
                void CreateCentralWidget ( );
                void ApplyWarmTheme ( );
                void ApplyThemeFromFile (const QString& filepath, bool persist_selection = true);
                QString GetThemeSettingsFile ( ) const;
                QString GetDefaultLoafDirectory ( ) const;
                bool CheckUnsavedChanges ( );

                QTabWidget *tab_widget_;

                HomeWidget *home_widget_;
                LoafEditorWidget *loaf_editor_;
                TextEditorWidget *text_editor_;
                ThemeEditorWidget *theme_editor_;
                LoafRuntimeWidget *runtime_widget_;
                AppBrowserWidget *app_browser_;
                LoafBrowserWidget *loaf_browser_;
                ThemeBrowserWidget *theme_browser_;

                QMenu *file_menu_;
                QMenu *edit_menu_;
                QMenu *view_menu_;
                QMenu *help_menu_;

                QToolBar *main_tool_bar_;
                QString current_theme_path_;
        };

    } // namespace GUI
} // namespace BreadBin

#endif // MAINWINDOW_H