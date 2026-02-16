#ifndef THEMEBROWSERWIDGET_H
#define THEMEBROWSERWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QColor>
#include <vector>

namespace BreadBin {
    namespace GUI {

        struct ThemeFileInfo {
            QString filepath;
            QString name;
            QString description;
            QString last_modified;
            QColor primary_colour;
            QColor secondary_colour;
            QString default_font_family;
            int default_font_size = 13;
            QString heading_font_family;
            int heading_font_size = 14;
            QString code_font_family;
            int code_font_size = 12;
        };

        class ThemeBrowserWidget : public QWidget {
            Q_OBJECT

            public:
                explicit ThemeBrowserWidget (QWidget *parent = nullptr);
                ~ThemeBrowserWidget ( );

                void RefreshThemeFiles ( );

                void SetSearchPaths (const QStringList& paths);

                QString GetSelectedTheme ( ) const;

            signals:
                void ThemeSelected (const QString& filepath);
                void ThemeApplied (const QString& filepath);

            private slots:
                void OnRefreshClicked ( );
                void OnSearchChanged (const QString& text);
                void OnThemeSelected ( );
                void OnApplyClicked ( );
                void OnDeleteClicked ( );

            private:
                void SetupUI ( );
                void ConnectSignals ( );
                void UpdateThemeList ( );
                void UpdateThemePreview (const ThemeFileInfo& info);
                void ScanForThemeFiles ( );
                ThemeFileInfo LoadThemeInfo (const QString& filepath);

                std::vector<ThemeFileInfo> theme_files_;
                std::vector<ThemeFileInfo> filtered_files_;
                QStringList search_paths_;
                QString current_theme_;

                QLineEdit *search_edit_;
                QListWidget *file_list_;
                QTextEdit *preview_text_;
                QPushButton *refresh_button_;
                QPushButton *apply_button_;
                QPushButton *delete_button_;
                QLabel *status_label_;
                QWidget *colour_preview_;
        };

    } // namespace GUI
} // namespace BreadBin

#endif // THEMEBROWSERWIDGET_H