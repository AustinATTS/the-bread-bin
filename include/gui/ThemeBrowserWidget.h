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
            QString lastModified;
            QColor primaryColour;
            QColor secondaryColour;
        };

        class ThemeBrowserWidget : public QWidget {
            Q_OBJECT

            public:
                explicit ThemeBrowserWidget(QWidget *parent = nullptr);
                ~ThemeBrowserWidget();

                void refreshThemeFiles();

                void setSearchPaths(const QStringList& paths);

                QString getSelectedTheme() const;

            signals:
                void themeSelected(const QString& filepath);
                void themeApplied(const QString& filepath);

            private slots:
                void onRefreshClicked();
                void onSearchChanged(const QString& text);
                void onThemeSelected();
                void onApplyClicked();
                void onDeleteClicked();

            private:
                void setupUI();
                void connectSignals();
                void updateThemeList();
                void updateThemePreview(const ThemeFileInfo& info);
                void scanForThemeFiles();
                ThemeFileInfo loadThemeInfo(const QString& filepath);

                std::vector<ThemeFileInfo> themeFiles_;
                std::vector<ThemeFileInfo> filteredFiles_;
                QStringList searchPaths_;
                QString currentTheme_;

                QLineEdit *searchEdit_;
                QListWidget *fileList_;
                QTextEdit *previewText_;
                QPushButton *refreshButton_;
                QPushButton *applyButton_;
                QPushButton *deleteButton_;
                QLabel *statusLabel_;
                QWidget *colourPreview_;
        };

    } // namespace GUI
} // namespace BreadBin

#endif // THEMEBROWSERWIDGET_H
