#ifndef THEMEEDITORWIDGET_H
#define THEMEEDITORWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QCheckBox>
#include <memory>
#include "ThemeEditor.h"

namespace BreadBin::GUI {
    class ThemeEditorWidget : public QWidget {
        Q_OBJECT

        public:
            explicit ThemeEditorWidget (QWidget *parent = nullptr);
            ~ThemeEditorWidget ( ) override;

            void NewTheme (const QString &name);
            bool LoadTheme (const QString &filepath);
            bool SaveTheme (const QString &filepath);
            bool SaveCurrentTheme ( );

        signals:
            void ThemeChanged ( );
            void ThemeApplied (const QString& filepath);

        private slots:
            void OnNewTheme ( );
            void OnLoadTheme ( );
            void OnSaveTheme ( );
            void OnElementSelected ( );
            void OnColourChanged ( );
            void OnFontChanged ( );
            void OnApplyTheme ( );
            void OnUseElementFontsToggled (bool enabled);
            void OnGlobalFontChanged ( );

        private:
            void SetupUI ( );
            void UpdateElementList ( );
            void UpdateColourPicker ( );
            void UpdateFontPicker ( );
            void ApplyGlobalFontToElements ( );
            void ConnectSignals ( );
            bool PromptAndSaveTheme ( );

            std::shared_ptr<ThemeEditor> editor_;
            QLineEdit *theme_name_edit_;
            QListWidget *element_list_;
            QPushButton *new_theme_button_;
            QPushButton *load_theme_button_;
            QPushButton *save_theme_button_;
            QPushButton *apply_theme_button_;
            QPushButton *colour_button_;
            QLabel *colour_preview_;
            QComboBox *font_combo_;
            QComboBox *font_size_combo_;
            QComboBox *global_font_combo_;
            QCheckBox *use_element_fonts_check_;
            QLabel *status_label_;
            QString current_theme_path_;
    };
} // namespace BreadBin::GUI

#endif // THEMEEDITORWIDGET_H