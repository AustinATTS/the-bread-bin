#ifndef THEMEEDITORWIDGET_H
#define THEMEEDITORWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <memory>
#include "ThemeEditor.h"

namespace BreadBin {
    namespace GUI {

        class ThemeEditorWidget : public QWidget {
            Q_OBJECT

            public:
                explicit ThemeEditorWidget(QWidget *parent = nullptr);
                ~ThemeEditorWidget();

                void newTheme(const QString &name);
                bool loadTheme(const QString &filepath);
                bool saveTheme(const QString &filepath);

            signals:
                void themeChanged();
                void themeApplied(const QString& filepath);

            private slots:
                void onNewTheme();
                void onLoadTheme();
                void onSaveTheme();
                void onElementSelected();
                void onColourChanged();
                void onFontChanged();
                void onApplyTheme();

            private:
                void setupUI();
                void updateElementList();
                void updateColourPicker();
                void updateFontPicker();
                void connectSignals();

                std::shared_ptr<ThemeEditor> editor_;

                QLineEdit *themeNameEdit_;
                QListWidget *elementList_;

                QPushButton *newThemeButton_;
                QPushButton *loadThemeButton_;
                QPushButton *saveThemeButton_;
                QPushButton *applyThemeButton_;

                QPushButton *colourButton_;
                QLabel *colourPreview_;

                QComboBox *fontCombo_;
                QComboBox *fontSizeCombo_;

                QLabel *statusLabel_;

                QString currentThemePath_;
        };

    } // namespace GUI
} // namespace BreadBin

#endif // THEMEEDITORWIDGET_H
