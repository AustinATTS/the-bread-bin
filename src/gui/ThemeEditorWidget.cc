#include "gui/ThemeEditorWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QInputDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <vector>

namespace BreadBin {
    namespace GUI {

    namespace {
        std::string SelectedThemeKey (const QListWidgetItem* item) {
            if (!item) {
                return "";
            }
            return item->data(Qt::UserRole).toString().toStdString();
        }
    }

    ThemeEditorWidget::ThemeEditorWidget (QWidget *parent)
        : QWidget(parent),
          editor_(std::make_shared<ThemeEditor>())
    {
        SetupUI();
        ConnectSignals();
        UpdateElementList();
    }

    ThemeEditorWidget::~ThemeEditorWidget ( ) {

    }

    void ThemeEditorWidget::SetupUI ( ) {
        QVBoxLayout *main_layout = new QVBoxLayout(this);
        main_layout->setContentsMargins(16, 16, 16, 16);
        main_layout->setSpacing(16);

        QGroupBox *info_group = new QGroupBox("🎨 Theme Information", this);
        QHBoxLayout *info_layout = new QHBoxLayout(info_group);
        info_layout->setContentsMargins(12, 20, 12, 12);
        info_layout->setSpacing(12);

        QLabel *name_label = new QLabel("Theme Name:", this);
        name_label->setStyleSheet("font-weight: 600; font-size: 13px;");
        theme_name_edit_ = new QLineEdit(this);
        theme_name_edit_->setPlaceholderText("My Theme");
        theme_name_edit_->setMinimumHeight(36);

        info_layout->addWidget(name_label);
        info_layout->addWidget(theme_name_edit_, 1);

        main_layout->addWidget(info_group);

        QHBoxLayout *editor_layout = new QHBoxLayout();
        editor_layout->setSpacing(16);

        QGroupBox *elements_group = new QGroupBox("🎯 UI Elements", this);
        elements_group->setMinimumWidth(250);
        QVBoxLayout *elements_layout = new QVBoxLayout(elements_group);
        elements_layout->setContentsMargins(12, 20, 12, 12);
        element_list_ = new QListWidget(this);
        element_list_->setMinimumHeight(200);
        elements_layout->addWidget(element_list_);
        editor_layout->addWidget(elements_group, 1);

        QGroupBox *props_group = new QGroupBox("⚙️ Properties", this);
        props_group->setMinimumWidth(300);
        QVBoxLayout *props_layout = new QVBoxLayout(props_group);
        props_layout->setContentsMargins(12, 20, 12, 12);
        props_layout->setSpacing(12);

        QLabel *colour_label = new QLabel("Colour:", this);
        colour_label->setStyleSheet("font-weight: 600; font-size: 13px;");
        colour_button_ = new QPushButton("🎨 Choose Color", this);
        colour_button_->setMinimumHeight(36);
        colour_preview_ = new QLabel(this);
        colour_preview_->setMinimumHeight(40);
        colour_preview_->setStyleSheet("background-color: #d4a574; border: 2px solid #e8dcc8; border-radius: 6px;");

        props_layout->addWidget(colour_label);
        props_layout->addWidget(colour_button_);
        props_layout->addWidget(colour_preview_);
        props_layout->addSpacing(12);

        QLabel *font_label = new QLabel("Font:", this);
        font_label->setStyleSheet("font-weight: 600; font-size: 13px;");
        font_combo_ = new QComboBox(this);
        font_combo_->addItems({"Arial", "Helvetica", "Times New Roman", "Courier", "Verdana"});
        font_combo_->setMinimumHeight(36);

        QLabel *size_label = new QLabel("Font Size:", this);
        size_label->setStyleSheet("font-weight: 600; font-size: 13px;");
        font_size_combo_ = new QComboBox(this);
        for (int i = 8; i <= 24; i += 2) {
            font_size_combo_->addItem(QString::number(i));
        }
        font_size_combo_->setCurrentText("12");
        font_size_combo_->setMinimumHeight(36);

        props_layout->addWidget(font_label);
        props_layout->addWidget(font_combo_);
        props_layout->addSpacing(8);
        props_layout->addWidget(size_label);
        props_layout->addWidget(font_size_combo_);
        props_layout->addStretch();

        editor_layout->addWidget(props_group, 1);
        main_layout->addLayout(editor_layout, 1);

        QHBoxLayout *button_layout = new QHBoxLayout();
        button_layout->setSpacing(8);

        new_theme_button_ = new QPushButton("➕ New Theme", this);
        new_theme_button_->setMinimumHeight(40);
        new_theme_button_->setMinimumWidth(120);

        load_theme_button_ = new QPushButton("📂 Load Theme", this);
        load_theme_button_->setMinimumHeight(40);
        load_theme_button_->setMinimumWidth(120);

        save_theme_button_ = new QPushButton("💾 Save Theme", this);
        save_theme_button_->setMinimumHeight(40);
        save_theme_button_->setMinimumWidth(120);

        apply_theme_button_ = new QPushButton("✨ Apply Theme", this);
        apply_theme_button_->setMinimumHeight(40);
        apply_theme_button_->setMinimumWidth(120);
        apply_theme_button_->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                          stop:0 #f0c080, stop:1 #e0a865);
                font-weight: 700;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                          stop:0 #e0a865, stop:1 #d09850);
            }
        )");

        button_layout->addWidget(new_theme_button_);
        button_layout->addWidget(load_theme_button_);
        button_layout->addWidget(save_theme_button_);
        button_layout->addWidget(apply_theme_button_);
        button_layout->addStretch();

        main_layout->addLayout(button_layout);

        status_label_ = new QLabel("Ready to create or edit themes", this);
        status_label_->setStyleSheet("color: #8b7a5e; font-size: 12px; padding: 8px;");
        main_layout->addWidget(status_label_);

        setLayout(main_layout);
    }

    void ThemeEditorWidget::ConnectSignals ( ) {
        connect(new_theme_button_, &QPushButton::clicked, this, &ThemeEditorWidget::OnNewTheme);
        connect(load_theme_button_, &QPushButton::clicked, this, &ThemeEditorWidget::OnLoadTheme);
        connect(save_theme_button_, &QPushButton::clicked, this, &ThemeEditorWidget::OnSaveTheme);
        connect(apply_theme_button_, &QPushButton::clicked, this, &ThemeEditorWidget::OnApplyTheme);
        connect(colour_button_, &QPushButton::clicked, this, &ThemeEditorWidget::OnColourChanged);
        connect(element_list_, &QListWidget::itemSelectionChanged, this, &ThemeEditorWidget::OnElementSelected);
        connect(font_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ThemeEditorWidget::OnFontChanged);
        connect(font_size_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ThemeEditorWidget::OnFontChanged);
    }

    void ThemeEditorWidget::NewTheme (const QString &name) {
        editor_->NewTheme(name.toStdString());
        theme_name_edit_->setText(name);
        UpdateElementList();
        status_label_->setText("Created new theme: " + name);
    }

    bool ThemeEditorWidget::LoadTheme (const QString &filepath) {
        if (editor_->LoadTheme(filepath.toStdString())) {
            theme_name_edit_->setText(QString::fromStdString(editor_->GetThemeName()));
            UpdateElementList();
            current_theme_path_ = filepath;
            status_label_->setText("Loaded theme: " + filepath);
            return true;
        }
        return false;
    }

    bool ThemeEditorWidget::SaveTheme (const QString &filepath) {
        editor_->SetThemeName(theme_name_edit_->text().toStdString());
        if (editor_->SaveTheme(filepath.toStdString())) {
            current_theme_path_ = filepath;
            status_label_->setText("Saved theme: " + filepath);
            return true;
        }
        return false;
    }

    void ThemeEditorWidget::OnNewTheme ( ) {
        QString name = theme_name_edit_->text();
        if (name.isEmpty()) {
            name = "My Theme";
        }
        NewTheme(name);
    }

    void ThemeEditorWidget::OnLoadTheme ( ) {
        const QString default_theme_dir =
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/BreadBin/themes";

        QString filepath = QFileDialog::getOpenFileName(
            this,
            "Load Theme",
            default_theme_dir,
            "Theme Files (*.theme);;All Files (*)"
        );

        if (!filepath.isEmpty()) {
            if (!LoadTheme(filepath)) {
                QMessageBox::warning(this, "Error", "Failed to load theme");
            }
        }
    }

    void ThemeEditorWidget::OnSaveTheme ( ) {
        const QString default_theme_dir =
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/BreadBin/themes";
        QDir().mkpath(default_theme_dir);

        QString suggested_name = theme_name_edit_->text().trimmed();
        if (suggested_name.isEmpty()) {
            suggested_name = "my_theme";
        }
        suggested_name.replace(' ', '_');

        QString filepath = QFileDialog::getSaveFileName(
            this,
            "Save Theme",
            default_theme_dir + "/" + suggested_name + ".theme",
            "Theme Files (*.theme);;All Files (*)"
        );

        if (!filepath.isEmpty()) {
            if (!filepath.endsWith(".theme")) {
                filepath += ".theme";
            }

            if (!SaveTheme(filepath)) {
                QMessageBox::warning(this, "Error", "Failed to save theme");
            }
        }
    }

    void ThemeEditorWidget::OnElementSelected ( ) {
        UpdateColourPicker();
        UpdateFontPicker();
    }

    void ThemeEditorWidget::OnColourChanged ( ) {
        QColor colour = QColorDialog::getColor(Qt::white, this, "Choose Color");
        if (colour.isValid()) {
            colour_preview_->setStyleSheet(
                QString("background-color: %1; border: 1px solid #000;").arg(colour.name())
            );

            QListWidgetItem *item = element_list_->currentItem();
            if (item) {
                const std::string key = SelectedThemeKey(item);
                editor_->SetColour(
                    key,
                    ThemeEditor::Colour(colour.red(), colour.green(), colour.blue(), colour.alpha())
                );
                status_label_->setText("Updated color for: " + item->text());
                emit ThemeChanged();
            }
        }
    }

    void ThemeEditorWidget::OnFontChanged ( ) {
        QListWidgetItem *item = element_list_->currentItem();
        if (item) {
            const std::string key = SelectedThemeKey(item);
            QString font_name = font_combo_->currentText();
            int font_size = font_size_combo_->currentText().toInt();

            editor_->SetFont(key, font_name.toStdString(), font_size);
            status_label_->setText("Updated font for: " + item->text());
            emit ThemeChanged();
        }
    }

    void ThemeEditorWidget::OnApplyTheme ( ) {
        if (current_theme_path_.isEmpty() || editor_->HasUnsavedChanges()) {
            OnSaveTheme();
        }

        if (current_theme_path_.isEmpty() || !QFileInfo::exists(current_theme_path_)) {
            QMessageBox::warning(this, "Error", "Theme must be saved before it can be applied.");
            return;
        }

        status_label_->setText("Applied theme: " + current_theme_path_);
        emit ThemeApplied(current_theme_path_);
        emit ThemeChanged();
    }

    void ThemeEditorWidget::UpdateElementList ( ) {
        element_list_->clear();

        struct ThemeElement {
            QString label;
            QString key;
        };

        const std::vector<ThemeElement> elements = {
            {"Window Background", "background"},
            {"Text/Foreground", "foreground"},
            {"Primary Accent", "primary"},
            {"Secondary Accent", "secondary"},
            {"Extra Accent", "accent"}
        };

        for (const auto& element : elements) {
            auto *list_item = new QListWidgetItem(element.label, element_list_);
            list_item->setData(Qt::UserRole, element.key);
        }
    }

    void ThemeEditorWidget::UpdateColourPicker ( ) {
        QListWidgetItem *item = element_list_->currentItem();
        if (item) {
            const ThemeEditor::Colour colour = editor_->GetColour(SelectedThemeKey(item));
            QColor q_colour(colour.red, colour.green, colour.blue, colour.alpha);
            colour_preview_->setStyleSheet(
                QString("background-color: %1; border: 1px solid #000;").arg(q_colour.name(QColor::HexArgb))
            );
        }
    }

    void ThemeEditorWidget::UpdateFontPicker ( ) {
        QListWidgetItem *item = element_list_->currentItem();
        if (item) {
            const std::string key = SelectedThemeKey(item);
            font_combo_->setCurrentText(QString::fromStdString(editor_->GetFont(key)));
            font_size_combo_->setCurrentText(QString::number(editor_->GetFontSize(key)));
        }
    }

    } // namespace GUI
} // namespace BreadBin
