#include "gui/ThemeBrowserWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <QPalette>
#include <QFile>
#include <QLabel>
#include <QLinearGradient>
#include <QBrush>
#include <fstream>
#include <sstream>
#include <QFontDatabase>

namespace BreadBin::GUI {
    namespace {
        QString CSSFontFamily (const QString& family) {
            QString escaped = family;
            escaped.replace("\\", "\\\\");
            escaped.replace("\"", "\\\"");
            return "\"" + escaped + "\"";
        }

        std::string trim (const std::string& value) {
            const auto first = value.find_first_not_of(" \t\r\n");
            if (first == std::string::npos) {
                return "";
            }
            const auto last = value.find_last_not_of(" \t\r\n");
            return value.substr(first, (last - first + 1));
        }

        bool ParseThemeEntry (const std::string& line, std::string& key, std::string& value) {
            if (line.empty() || line[0] == '#') {
                return false;
            }

            const auto seperator = line.find_first_of(":=");
            if (seperator == std::string::npos) {
                return false;
            }

            key = trim(line.substr(0, seperator));
            value = trim(line.substr(seperator + 1));
            return !key.empty();
        }

        QColor ParseThemeColour (const std::string& raw_value) {
            const QString value = QString::fromStdString(raw_value).trimmed();
            QColor direct_colour(value);
            if (direct_colour.isValid()) {
                return direct_colour;
            }

            std::istringstream ss(raw_value);
            int red, green, blue, alpha = 255;
            if (ss >> red >> green >> blue) {
                if (!(ss >> alpha)) {
                    alpha = 255;
                }
                return QColor(red, green, blue, alpha);
            }

            return QColor();
        }
    }

    ThemeBrowserWidget::ThemeBrowserWidget (QWidget *parent)
        : QWidget(parent)
    {
        QStringList default_paths;
        default_paths << QDir::homePath() + "/.breadbin/themes";
        default_paths << QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/BreadBin/themes";
        default_paths << QDir::currentPath();
        SetSearchPaths(default_paths);

        SetupUI();
        ConnectSignals();
        RefreshThemeFiles();
    }

    ThemeBrowserWidget::~ThemeBrowserWidget ( ) {

    }

    void ThemeBrowserWidget::SetupUI ( ) {
        QVBoxLayout *main_layout = new QVBoxLayout(this);
        main_layout->setContentsMargins(16, 16, 16, 16);
        main_layout->setSpacing(16);

        QGroupBox *search_group = new QGroupBox("🔍 Search", this);
        QHBoxLayout *search_layout = new QHBoxLayout(search_group);
        search_layout->setContentsMargins(12, 20, 12, 12);
        search_layout->setSpacing(12);

        QLabel *search_label = new QLabel("Filter:", this);
        search_label->setStyleSheet("font-weight: 600; font-size: 13px; min-width: 60px;");
        search_edit_ = new QLineEdit(this);
        search_edit_->setPlaceholderText("Search themes...");
        search_edit_->setMinimumHeight(36);
        search_layout->addWidget(search_label);
        search_layout->addWidget(search_edit_, 1);

        main_layout->addWidget(search_group);

        QHBoxLayout *content_layout = new QHBoxLayout();
        content_layout->setSpacing(16);

        QGroupBox *list_group = new QGroupBox("🎨 Available Themes", this);
        list_group->setMinimumWidth(300);
        QVBoxLayout *list_layout = new QVBoxLayout(list_group);
        list_layout->setContentsMargins(12, 20, 12, 12);
        list_layout->setSpacing(12);

        file_list_ = new QListWidget(this);
        file_list_->setMinimumHeight(250);
        list_layout->addWidget(file_list_, 1);

        QHBoxLayout *button_layout = new QHBoxLayout();
        button_layout->setSpacing(8);
        refresh_button_ = new QPushButton("🔄 Refresh", this);
        refresh_button_->setMinimumHeight(40);
        refresh_button_->setMinimumWidth(100);
        apply_button_ = new QPushButton("✨ Apply", this);
        apply_button_->setMinimumHeight(40);
        apply_button_->setMinimumWidth(100);
        apply_button_->setEnabled(false);
        apply_button_->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                          stop:0 #f0c080, stop:1 #e0a865);
                font-weight: 700;
            }
            QPushButton:hover {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                          stop:0 #e0a865, stop:1 #d09850);
            }
            QPushButton:disabled {
                background-color: #e8dcc8;
                color: #a89580;
            }
        )");
        delete_button_ = new QPushButton("🗑️ Delete", this);
        delete_button_->setMinimumHeight(40);
        delete_button_->setMinimumWidth(100);
        delete_button_->setEnabled(false);
        button_layout->addWidget(refresh_button_);
        button_layout->addWidget(apply_button_);
        button_layout->addWidget(delete_button_);
        button_layout->addStretch();
        list_layout->addLayout(button_layout);

        content_layout->addWidget(list_group, 1);

        QGroupBox *preview_group = new QGroupBox("👁️ Theme Preview", this);
        preview_group->setMinimumWidth(350);
        QVBoxLayout *preview_layout = new QVBoxLayout(preview_group);
        preview_layout->setContentsMargins(12, 20, 12, 12);
        preview_layout->setSpacing(12);

        colour_preview_ = new QWidget(this);
        colour_preview_->setMinimumHeight(80);
        colour_preview_->setAutoFillBackground(true);
        colour_preview_->setStyleSheet(R"(
            QWidget {
                border: 2px solid #e8dcc8;
                border-radius: 6px;
            }
        )");
        preview_layout->addWidget(colour_preview_);

        preview_text_ = new QTextEdit(this);
        preview_text_->setReadOnly(true);
        preview_text_->setMinimumHeight(200);
        preview_text_->setStyleSheet(R"(
            QTextEdit {
                background-color: #fafafa;
                border: 2px solid #e8dcc8;
                border-radius: 6px;
                padding: 12px;
                color: #2d1f0f;
                font-size: 13px;
            }
        )");
        preview_layout->addWidget(preview_text_, 1);

        content_layout->addWidget(preview_group, 1);

        main_layout->addLayout(content_layout, 1);

        status_label_ = new QLabel("Ready", this);
        status_label_->setStyleSheet("color: #8b7a5e; font-size: 12px; padding: 8px;");
        main_layout->addWidget(status_label_);

        setLayout(main_layout);
    }

    void ThemeBrowserWidget::ConnectSignals () {
        connect(refresh_button_, &QPushButton::clicked, this, &ThemeBrowserWidget::OnRefreshClicked);
        connect(search_edit_, &QLineEdit::textChanged, this, &ThemeBrowserWidget::OnSearchChanged);
        connect(file_list_, &QListWidget::itemSelectionChanged, this, &ThemeBrowserWidget::OnThemeSelected);
        connect(file_list_, &QListWidget::itemDoubleClicked, [this](QListWidgetItem*) { OnApplyClicked(); });
        connect(apply_button_, &QPushButton::clicked, this, &ThemeBrowserWidget::OnApplyClicked);
        connect(delete_button_, &QPushButton::clicked, this, &ThemeBrowserWidget::OnDeleteClicked);
    }

    void ThemeBrowserWidget::SetSearchPaths (const QStringList& paths) {
        search_paths_ = paths;
    }

    void ThemeBrowserWidget::RefreshThemeFiles ( ) {
        ScanForThemeFiles();
        UpdateThemeList();
    }

    QString ThemeBrowserWidget::GetSelectedTheme ( ) const {
        QListWidgetItem *item = file_list_->currentItem();
        if (!item) {
            return QString();
        }

        int index = file_list_->currentRow();
        if (index >= 0 && index < filtered_files_.size()) {
            return filtered_files_[index].filepath;
        }

        return QString();
    }

    void ThemeBrowserWidget::ScanForThemeFiles ( ) {
        theme_files_.clear();

        for (const QString& path : search_paths_) {
            QDir dir(path);
            if (!dir.exists()) {
                continue;
            }

            QStringList filters;
            filters << "*.theme" << "*.qss";

            QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
            for (const QFileInfo& file_info : files) {
                ThemeFileInfo info = LoadThemeInfo(file_info.absoluteFilePath());
                theme_files_.push_back(info);
            }
        }

        status_label_->setText(QString("Found %1 theme files").arg(theme_files_.size()));
    }

    ThemeFileInfo ThemeBrowserWidget::LoadThemeInfo (const QString& filepath) {
        ThemeFileInfo info;
        info.filepath = filepath;

        QFileInfo fileInfo(filepath);
        info.last_modified = fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss");

        std::ifstream file(filepath.toStdString());
        if (file.is_open()) {
            std::string line;

            std::string key;
            std::string value;
            while (std::getline(file, line)) {
                if (!ParseThemeEntry(line, key, value)) {
                    continue;
                }

                if (key == "NAME") {
                    info.name = QString::fromStdString(value).trimmed();
                }
                else {
                    if (key == "DESCRIPTION") {
                        info.description = QString::fromStdString(value).trimmed();
                    }
                    else {
                        if (key == "PRIMARY_COLOUR") {
                            QColor colour = ParseThemeColour(value);
                            if (colour.isValid()) {
                                info.primary_colour = colour;
                            }
                        }
                        else {
                            if (key == "SECONDARY_COLOUR") {
                                QColor colour = ParseThemeColour(value);
                                if (colour.isValid()) {
                                    info.secondary_colour = colour;
                                }
                            }
                            else {
                                if (key == "FONT_default" || key == "FONT_global") {
                                    const QStringList parts = QString::fromStdString(value).trimmed().split(',');
                                    if (!parts.isEmpty()) {
                                        info.default_font_family = parts[0].trimmed();
                                    }
                                    if (parts.size() > 1) {
                                        info.default_font_size = parts[1].trimmed().toInt();
                                    }
                                }
                                else {
                                    if (key == "FONT_heading") {
                                        const QStringList parts = QString::fromStdString(value).trimmed().split(',');
                                        if (!parts.isEmpty()) {
                                            info.heading_font_family = parts[0].trimmed();
                                        }
                                        if (parts.size() > 1) {
                                            info.heading_font_size = parts[1].trimmed().toInt();
                                        }
                                    }
                                    else {
                                        if (key == "FONT_code") {
                                            const QStringList parts = QString::fromStdString(value).trimmed().split(',');
                                            if (!parts.isEmpty()) {
                                                info.code_font_family = parts[0].trimmed();
                                            }
                                            if (parts.size() > 1) {
                                                info.code_font_size = parts[1].trimmed().toInt();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (info.name.isEmpty()) {
            info.name = fileInfo.baseName();
        }

        if (!info.primary_colour.isValid()) {
            info.primary_colour = QColor("#d4a574");
        }
        if (!info.secondary_colour.isValid()) {
            info.secondary_colour = QColor("#f5f0e8");
        }

        const QFontDatabase database;
        const QString default_ui_font = database.families().isEmpty()
            ? QStringLiteral("Sans Serif")
            : database.families().first();

        if (info.default_font_family.isEmpty()) {
            info.default_font_family = default_ui_font;
        }
        if (info.heading_font_family.isEmpty()) {
            info.heading_font_family = info.default_font_family;
        }
        if (info.code_font_family.isEmpty()) {
            info.code_font_family = QStringLiteral("Monospace");
        }

        return info;
    }

    void ThemeBrowserWidget::OnRefreshClicked ( ) {
        RefreshThemeFiles();
    }

    void ThemeBrowserWidget::OnSearchChanged (const QString& text) {
        UpdateThemeList();
    }

    void ThemeBrowserWidget::OnThemeSelected ( ) {
        QListWidgetItem *item = file_list_->currentItem();
        if (!item) {
            apply_button_->setEnabled(false);
            delete_button_->setEnabled(false);
            preview_text_->clear();

            QPalette pal = colour_preview_->palette();
            pal.setColor(QPalette::Window, Qt::white);
            colour_preview_->setPalette(pal);
            return;
        }

        apply_button_->setEnabled(true);
        delete_button_->setEnabled(true);

        int index = file_list_->currentRow();
        if (index >= 0 && index < filtered_files_.size()) {
            UpdateThemePreview(filtered_files_[index]);
            emit ThemeSelected(filtered_files_[index].filepath);
        }
    }

    void ThemeBrowserWidget::OnApplyClicked ( ) {
        QListWidgetItem *item = file_list_->currentItem();
        if (!item) {
            return;
        }

        int index = file_list_->currentRow();
        if (index >= 0 && index < filtered_files_.size()) {
            current_theme_ = filtered_files_[index].filepath;
            emit ThemeApplied(current_theme_);
            status_label_->setText("Applied theme: " + filtered_files_[index].name);
        }
    }

    void ThemeBrowserWidget::OnDeleteClicked ( ) {
        QListWidgetItem *item = file_list_->currentItem();
        if (!item) {
            return;
        }

        int index = file_list_->currentRow ( );
        if (index >= 0 && index < filtered_files_.size()) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "Confirm Delete",
                "Are you sure you want to delete theme '" + filtered_files_[index].name + "'?",
                QMessageBox::Yes | QMessageBox::No
            );

            if (reply == QMessageBox::Yes) {
                QFile file(filtered_files_[index].filepath);
                if (file.remove()) {
                    status_label_->setText("Deleted theme: " + filtered_files_[index].name);
                    RefreshThemeFiles();
                }
                else {
                    QMessageBox::warning(this, "Error", "Failed to delete theme file");
                }
            }
        }
    }

    void ThemeBrowserWidget::UpdateThemeList ( ) {
        file_list_->clear();
        filtered_files_.clear();

        QString search_text = search_edit_->text().toLower();

        for (const auto& theme : theme_files_) {
            if (!search_text.isEmpty()) {
                QString name = theme.name.toLower();
                if (!name.contains(search_text)) {
                    continue;
                }
            }

            filtered_files_.push_back(theme);
        }

        for (const auto& theme : filtered_files_) {
            file_list_->addItem(theme.name);
        }

        status_label_->setText(QString("Showing %1 of %2 themes")
                              .arg(filtered_files_.size())
                              .arg(theme_files_.size()));
    }

    void ThemeBrowserWidget::UpdateThemePreview (const ThemeFileInfo& info) {
        QPalette pal = colour_preview_->palette();
        QLinearGradient gradient(0, 0, colour_preview_->width(), 0);
        gradient.setColorAt(0, info.primary_colour);
        gradient.setColorAt(1, info.secondary_colour);

        QBrush brush(gradient);
        pal.setBrush(QPalette::Window, brush);
        colour_preview_->setPalette(pal);

        QString preview;

        preview += "<h3>" + info.name + "</h3>";

        if (!info.description.isEmpty()) {
            preview += "<p><b>Description:</b> " + info.description + "</p>";
        }

        preview += "<p><b>File:</b> <code>" + info.filepath + "</code></p>";
        preview += "<p><b>Last Modified:</b> " + info.last_modified + "</p>";

        preview += "<hr><h4>Color Scheme:</h4>";
        preview += "<p><b>Primary Color:</b> <span style='background-color:" +
                   info.primary_colour.name() + ";padding:2px 10px;'>" +
                   info.primary_colour.name() + "</span></p>";
        preview += "<p><b>Secondary Color:</b> <span style='background-color:" +
                   info.secondary_colour.name() + ";padding:2px 10px;'>" +
                   info.secondary_colour.name() + "</span></p>";

        preview += "<hr><h4>Typography:</h4>";
        preview += "<p><b>Body:</b> " + info.default_font_family + " (" + QString::number(info.default_font_size) + "pt)</p>";
        preview += "<p><b>Heading:</b> " + info.heading_font_family + " (" + QString::number(info.heading_font_size) + "pt)</p>";
        preview += "<p><b>Code:</b> " + info.code_font_family + " (" + QString::number(info.code_font_size) + "pt)</p>";

        preview += "<div style='padding: 10px; border: 1px solid #e8dcc8; border-radius: 6px; background-color: #fff;'>";
        preview += "<div style='font-family:" + CSSFontFamily(info.heading_font_family) + "; font-size:" + QString::number(info.heading_font_size) + "pt; font-weight:700;'>"
            "Heading Preview: The Bread Bin</div>";
        preview += "<div style='margin-top:8px; font-family:" + CSSFontFamily(info.default_font_family) + "; font-size:" + QString::number(info.default_font_size) + "pt;'>"
            "Body Preview: Pack my box with five dozen liquor jugs.</div>";
        preview += "<div style='margin-top:8px; font-family:" + CSSFontFamily(info.code_font_family) + "; font-size:" + QString::number(info.code_font_size) + "pt;'>"
            "Code Preview: for (int i = 0; i < 5; ++i) { bake(); }</div>";
        preview += "</div>";

        std::ifstream file(info.filepath.toStdString());
        if (file.is_open()) {
            preview += "<hr><h4>Theme Definition:</h4>";
            preview += "<pre>";

            std::string line;
            int lineCount = 0;
            while (std::getline(file, line) && lineCount < 15) {
                preview += QString::fromStdString(line).toHtmlEscaped() + "\n";
                lineCount++;
            }

            if (file.good()) {
                preview += "...\n(file continues)";
            }

            preview += "</pre>";
        }

        preview_text_->setHtml(preview);
    }
} // namespace BreadBin::GUI