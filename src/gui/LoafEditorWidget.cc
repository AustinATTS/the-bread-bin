#include "gui/LoafEditorWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QStandardPaths>
#include <QCheckBox>
#include <QDir>
#include <QFileInfo>
#include <sstream>
#include <algorithm>
#include "AppDiscovery.h"

namespace BreadBin {
    namespace GUI {

    LoafEditorWidget::LoafEditorWidget (QWidget *parent)
        : QWidget(parent),
          editor_(std::make_shared<LoafEditor>())
    {
        SetupUI();
        ConnectSignals();

        // Initialize with a default empty loaf so users can start adding items immediately
        editor_->NewLoaf("Untitled Loaf");
        UpdateLoafInfo();
        status_label_->setText("Ready to add items to your loaf");
    }

    LoafEditorWidget::~LoafEditorWidget ( ) {
    }

    void LoafEditorWidget::SetupUI ( ) {
        QVBoxLayout *main_layout = new QVBoxLayout(this);
        main_layout->setContentsMargins(16, 16, 16, 16);
        main_layout->setSpacing(16);

        QGroupBox *info_group = new QGroupBox("📋 Loaf Information", this);
        QVBoxLayout *info_layout = new QVBoxLayout(info_group);
        info_layout->setContentsMargins(12, 20, 12, 12);
        info_layout->setSpacing(12);

        QLabel *name_label = new QLabel("Name:", this);
        name_label->setStyleSheet("font-weight: 600; font-size: 13px;");
        name_edit_ = new QLineEdit(this);
        name_edit_->setPlaceholderText("Enter a descriptive name for this loaf");
        name_edit_->setToolTip("The name of this loaf (e.g., 'Development Environment', 'Media Suite')");
        name_edit_->setMinimumHeight(36);
        info_layout->addWidget(name_label);
        info_layout->addWidget(name_edit_);

        QLabel *desc_label = new QLabel("Description:", this);
        desc_label->setStyleSheet("font-weight: 600; font-size: 13px;");
        description_edit_ = new QTextEdit(this);
        description_edit_->setMaximumHeight(100);
        description_edit_->setMinimumHeight(80);
        description_edit_->setPlaceholderText("Enter a description of what this loaf contains");
        description_edit_->setToolTip("A brief description of the purpose and contents of this loaf");
        info_layout->addWidget(desc_label);
        info_layout->addWidget(description_edit_);

        main_layout->addWidget(info_group);

        QGroupBox *items_group = new QGroupBox("📦 Loaf Items", this);
        QVBoxLayout *items_layout = new QVBoxLayout(items_group);
        items_layout->setContentsMargins(12, 20, 12, 12);
        items_layout->setSpacing(12);

        item_list_ = new QListWidget(this);
        item_list_->setToolTip("List of all items in this loaf. Select an item and click 'Configure' to set up dependencies and custom settings.");
        item_list_->setMinimumHeight(200);
        items_layout->addWidget(item_list_, 1);  // Takes most space

        QHBoxLayout *button_layout = new QHBoxLayout();
        button_layout->setSpacing(8);

        add_app_button_ = new QPushButton("➕ App", this);
        add_app_button_->setToolTip("Browse for an application executable to add to this loaf");
        add_app_button_->setMinimumWidth(80);

        add_file_button_ = new QPushButton("📄 File", this);
        add_file_button_->setToolTip("Browse for a file to add to this loaf");
        add_file_button_->setMinimumWidth(80);

        add_script_button_ = new QPushButton("📜 Script", this);
        add_script_button_->setToolTip("Browse for a script file (*.sh, *.bat, *.py, etc.)");
        add_script_button_->setMinimumWidth(80);

        add_config_button_ = new QPushButton("⚙️ Config", this);
        add_config_button_->setToolTip("Browse for a configuration file (*.conf, *.ini, *.json, etc.)");
        add_config_button_->setMinimumWidth(80);

        add_web_page_button_ = new QPushButton("🌐 Web", this);
        add_web_page_button_->setToolTip("Add a web page by entering its URL");
        add_web_page_button_->setMinimumWidth(80);

        button_layout->addWidget(add_app_button_);
        button_layout->addWidget(add_file_button_);
        button_layout->addWidget(add_script_button_);
        button_layout->addWidget(add_config_button_);
        button_layout->addWidget(add_web_page_button_);
        button_layout->addStretch();

        QHBoxLayout *action_layout = new QHBoxLayout();
        action_layout->setSpacing(8);

        remove_button_ = new QPushButton("🗑️ Remove", this);
        remove_button_->setToolTip("Remove the selected item from this loaf");
        remove_button_->setMinimumWidth(100);

        configure_button_ = new QPushButton("⚙️ Configure", this);
        configure_button_->setToolTip("Configure dependencies and custom settings for the selected item");
        configure_button_->setEnabled(false);  // Disabled until item is selected
        configure_button_->setMinimumWidth(100);

        action_layout->addWidget(remove_button_);
        action_layout->addWidget(configure_button_);
        action_layout->addStretch();

        items_layout->addLayout(button_layout);
        items_layout->addLayout(action_layout);
        main_layout->addWidget(items_group, 1);  // Takes most space

        status_label_ = new QLabel("Ready to create a new loaf", this);
        status_label_->setStyleSheet("color: #8b7a5e; font-size: 12px; padding: 8px;");
        main_layout->addWidget(status_label_);

        setLayout(main_layout);
    }

    void LoafEditorWidget::ConnectSignals ( ) {
        connect(name_edit_, &QLineEdit::textChanged, this, &LoafEditorWidget::OnNameChanged);
        connect(description_edit_, &QTextEdit::textChanged, this, &LoafEditorWidget::OnDescriptionChanged);
        connect(add_app_button_, &QPushButton::clicked, this, &LoafEditorWidget::OnAddApplication);
        connect(add_file_button_, &QPushButton::clicked, this, &LoafEditorWidget::OnAddFile);
        connect(add_script_button_, &QPushButton::clicked, this, &LoafEditorWidget::OnAddScript);
        connect(add_config_button_, &QPushButton::clicked, this, &LoafEditorWidget::OnAddConfig);
        connect(add_web_page_button_, &QPushButton::clicked, this, &LoafEditorWidget::OnAddWebPage);
        connect(remove_button_, &QPushButton::clicked, this, &LoafEditorWidget::OnRemoveItem);
        connect(configure_button_, &QPushButton::clicked, this, &LoafEditorWidget::OnConfigureItem);
        connect(item_list_, &QListWidget::itemSelectionChanged, this, &LoafEditorWidget::OnItemSelected);
    }

    QString LoafEditorWidget::GenerateItemId (const QString& name) const {
        QString id = name.toLower();

        QString sanitized;
        for (const QChar& ch : id) {
            if (ch.isLetterOrNumber()) {
                sanitized += ch;
            }
            else {
                sanitized += '_';
            }
        }

        QString result;
        bool last_was_underscore = false;
        for (const QChar& ch : sanitized) {
            if (ch == '_') {
                if (!last_was_underscore) {
                    result += ch;
                    last_was_underscore = true;
                }
            }
            else {
                result += ch;
                last_was_underscore = false;
            }
        }
        sanitized = result;

        while (sanitized.startsWith('_')) {
            sanitized = sanitized.mid(1);
        }
        while (sanitized.endsWith('_')) {
            sanitized = sanitized.left(sanitized.length() - 1);
        }

        if (sanitized.isEmpty()) {
            sanitized = "item";
        }

        if (sanitized.length() > 50) {
            sanitized = sanitized.left(50);
        }

        return sanitized;
    }

    void LoafEditorWidget::NewLoaf ( ) {
        bool ok;
        QString name = QInputDialog::getText(
            this,
            "New Loaf",
            "Enter loaf name:",
            QLineEdit::Normal,
            "My Loaf",
            &ok
        );

        if (ok && !name.isEmpty()) {
            editor_->NewLoaf(name.toStdString());
            current_file_path_.clear(); // New loaf has no path yet
            UpdateLoafInfo();
            status_label_->setText("Created new loaf: " + name);
            emit LoafModified();
        }
    }

    bool LoafEditorWidget::OpenLoaf (const QString &filepath) {
        if (editor_->OpenLoaf(filepath.toStdString())) {
            current_file_path_ = filepath; // Track the file path
            UpdateLoafInfo();
            RefreshItemList();
            status_label_->setText("Opened: " + filepath);
            return true;
        }
        return false;
    }

    bool LoafEditorWidget::SaveLoaf (const QString &filepath) {
        if (editor_->SaveLoaf(filepath.toStdString())) {
            current_file_path_ = filepath; // Track the file path
            status_label_->setText("Saved: " + filepath);
            return true;
        }
        return false;
    }

    bool LoafEditorWidget::SaveLoaf ( ) {
        if (current_file_path_.isEmpty()) {
            return false; // No path to save to
        }
        return SaveLoaf(current_file_path_);
    }

    QString LoafEditorWidget::GetCurrentFilePath ( ) const {
        return current_file_path_;
    }

    bool LoafEditorWidget::HasUnsavedChanges ( ) const {
        return editor_->HasUnsavedChanges();
    }

    std::shared_ptr<Loaf> LoafEditorWidget::GetCurrentLoaf ( ) const {
        return editor_->GetCurrentLoaf();
    }

    void LoafEditorWidget::OnAddApplication ( ) {
        QDialog dialog(this);
        dialog.setWindowTitle("Add Application");
        dialog.setMinimumWidth(620);

        QVBoxLayout *layout = new QVBoxLayout(&dialog);

        QLabel *label = new QLabel("Select an application or enter path manually:", &dialog);
        layout->addWidget(label);

        QGroupBox *select_group = new QGroupBox("Select Installed Application", &dialog);
        QVBoxLayout *select_layout = new QVBoxLayout(select_group);

        QLineEdit *search_edit = new QLineEdit(&dialog);
        search_edit->setPlaceholderText("Search discovered applications...");
        select_layout->addWidget(search_edit);

        QComboBox *app_combo = new QComboBox(&dialog);
        app_combo->setMinimumContentsLength(40);
        app_combo->addItem("-- Browse for application --", "");

        AppDiscovery discovery;
        discovery.ScanSystem();
        auto apps = discovery.GetApplications();
        std::sort(apps.begin(), apps.end(), [](const AppInfo& a, const AppInfo& b) {
            return a.name < b.name;
        });

        for (const auto& app : apps) {
            if (app.name.empty() || app.executable.empty()) {
                continue;
            }

            QString label = QString::fromStdString(app.name);
            if (!app.category.empty()) {
                label += " (" + QString::fromStdString(app.category) + ")";
            }
            app_combo->addItem(label, QString::fromStdString(app.executable));
        }

        select_layout->addWidget(new QLabel("Application:", &dialog));
        select_layout->addWidget(app_combo);
        layout->addWidget(select_group);

        QGroupBox *browse_group = new QGroupBox("Or Browse for Application", &dialog);
        QVBoxLayout *browse_layout = new QVBoxLayout(browse_group);

        QLineEdit *path_edit = new QLineEdit(&dialog);
        QPushButton *browse_button = new QPushButton("Browse...", &dialog);

        QHBoxLayout *browse_input_layout = new QHBoxLayout();
        browse_input_layout->addWidget(path_edit);
        browse_input_layout->addWidget(browse_button);

        browse_layout->addWidget(new QLabel("Path:", &dialog));
        browse_layout->addLayout(browse_input_layout);
        layout->addWidget(browse_group);

        QLabel *name_label = new QLabel("Display Name:", &dialog);
        QLineEdit *name_edit = new QLineEdit(&dialog);
        layout->addWidget(name_label);
        layout->addWidget(name_edit);

        QDialogButtonBox *buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        layout->addWidget(buttons);

        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        connect(browse_button, &QPushButton::clicked, [&]() {
            QString path = QFileDialog::getOpenFileName(
                &dialog, "Select Application", QDir::homePath(),
    #ifdef _WIN32
                "Executables (*.exe);;All Files (*.*)"
    #else
                "All Files (*)"
    #endif
            );
            if (!path.isEmpty()) {
                path_edit->setText(path);
                if (name_edit->text().isEmpty()) {
                    QFileInfo info(path);
                    name_edit->setText(info.completeBaseName());
                }
            }
        });

        connect(app_combo, qOverload<int>(&QComboBox::currentIndexChanged), [&](int) {
            const QString selected_path = app_combo->currentData().toString();
            if (selected_path.isEmpty()) {
                return;
            }

            path_edit->setText(selected_path);
            QFileInfo info(selected_path);
            name_edit->setText(info.completeBaseName().isEmpty() ? app_combo->currentText() : info.completeBaseName());
        });

        connect(search_edit, &QLineEdit::textChanged, [&](const QString& text) {
            app_combo->clear();
            app_combo->addItem("-- Browse for application --", "");

            const std::string query = text.trimmed().toStdString();
            const auto filtered_apps = discovery.SearchApplications(query);

            for (const auto& app : filtered_apps) {
                if (app.name.empty() || app.executable.empty()) {
                    continue;
                }

                QString label = QString::fromStdString(app.name);
                if (!app.category.empty()) {
                    label += " (" + QString::fromStdString(app.category) + ")";
                }
                app_combo->addItem(label, QString::fromStdString(app.executable));
            }
        });

        if (dialog.exec() == QDialog::Accepted) {
            QString path = path_edit->text();
            if (path.isEmpty()) {
                path = app_combo->currentData().toString();
            }
            QString name = name_edit->text();

            if (path.isEmpty()) {
                QMessageBox::warning(this, "Error", "Please select or enter an application path");
                return;
            }

            if (name.isEmpty()) {
                QFileInfo info(path);
                name = info.completeBaseName();
            }

            QString id = GenerateItemId(name);

            if (editor_->AddApplication(id.toStdString(), name.toStdString(), path.toStdString())) {
                RefreshItemList();
                status_label_->setText("Added application: " + name);
                emit LoafModified();
            }
        }
    }

    void LoafEditorWidget::OnAddFile ( ) {
        QString path = QFileDialog::getOpenFileName(
            this, "Select File", QDir::homePath(),
            "All Files (*)"
        );

        if (path.isEmpty()) {
            return;
        }

        QFileInfo info(path);
        QString name = info.fileName();

        bool ok;
        name = QInputDialog::getText(this, "Add File", "Display Name:", QLineEdit::Normal, name, &ok);
        if (!ok || name.isEmpty()) {
            name = info.fileName();
        }

        QString id = GenerateItemId(name);

        if (editor_->AddFile(id.toStdString(), name.toStdString(), path.toStdString())) {
            RefreshItemList();
            status_label_->setText("Added file: " + name);
            emit LoafModified();
        }
    }

    void LoafEditorWidget::OnAddScript ( ) {
        QDialog dialog(this);
        dialog.setWindowTitle("Add Script");
        dialog.setMinimumWidth(400);

        QVBoxLayout *layout = new QVBoxLayout(&dialog);

        QLabel *label = new QLabel("Choose an option:", &dialog);
        layout->addWidget(label);

        QPushButton *browse_btn = new QPushButton("Browse for Existing Script", &dialog);
        QPushButton *create_btn = new QPushButton("Create New Script", &dialog);
        QPushButton *cancel_btn = new QPushButton("Cancel", &dialog);

        layout->addWidget(browse_btn);
        layout->addWidget(create_btn);
        layout->addWidget(cancel_btn);

        bool should_browse = false;
        bool should_create = false;

        connect(browse_btn, &QPushButton::clicked, [&]() {
            should_browse = true;
            dialog.accept();
        });

        connect(create_btn, &QPushButton::clicked, [&]() {
            should_create = true;
            dialog.accept();
        });

        connect(cancel_btn, &QPushButton::clicked, &dialog, &QDialog::reject);

        if (dialog.exec() != QDialog::Accepted) {
            return;
        }

        if (should_create) {
            emit CreateNewScriptRequested();
            return;
        }

        if (!should_browse) {
            return;
        }

        QString path = QFileDialog::getOpenFileName(
            this, "Select Script", QDir::homePath(),
    #ifdef _WIN32
            "Scripts (*.bat *.cmd *.ps1 *.py *.sh);;All Files (*.*)"
    #else
            "Scripts (*.sh *.py *.rb *.pl);;All Files (*)"
    #endif
        );

        if (path.isEmpty()) {
            return;
        }

        QFileInfo info(path);
        QString name = info.fileName();

        bool ok;
        name = QInputDialog::getText(this, "Add Script", "Display Name:", QLineEdit::Normal, name, &ok);
        if (!ok || name.isEmpty()) {
            name = info.fileName();
        }

        QString id = GenerateItemId(name);

        if (editor_->AddScript(id.toStdString(), name.toStdString(), path.toStdString())) {
            RefreshItemList();
            status_label_->setText("Added script: " + name);
            emit LoafModified();
        }
    }

    void LoafEditorWidget::OnAddConfig ( ) {
        QString path = QFileDialog::getOpenFileName(
            this, "Select Config File", QDir::homePath(),
            "Config Files (*.conf *.cfg *.ini *.json *.xml *.yaml *.yml *.toml);;All Files (*)"
        );

        if (path.isEmpty()) {
            return;
        }

        QFileInfo info(path);
        QString name = info.fileName();

        bool ok;
        name = QInputDialog::getText(this, "Add Config", "Display Name:", QLineEdit::Normal, name, &ok);
        if (!ok || name.isEmpty()) {
            name = info.fileName();
        }

        QString id = GenerateItemId(name);

        if (editor_->AddConfig(id.toStdString(), name.toStdString(), path.toStdString())) {
            RefreshItemList();
            status_label_->setText("Added config: " + name);
            emit LoafModified();
        }
    }

    void LoafEditorWidget::OnAddWebPage ( ) {
        bool ok;
        QString url = QInputDialog::getText(
            this, "Add Web Page",
            "Enter URL (e.g., https://example.com):",
            QLineEdit::Normal, "https://", &ok
        );

        if (!ok || url.isEmpty()) {
            return;
        }

        if (!url.startsWith("http://") && !url.startsWith("https://")) {
            QMessageBox::warning(this, "Invalid URL", "URL must start with http:// or https://");
            return;
        }

        QString name = QInputDialog::getText(
            this, "Add Web Page",
            "Display Name:",
            QLineEdit::Normal, url, &ok
        );

        if (!ok || name.isEmpty()) {
            name = url;
        }

        QString id = GenerateItemId(name);

        if (editor_->AddWebPage(id.toStdString(), name.toStdString(), url.toStdString())) {
            RefreshItemList();
            status_label_->setText("Added web page: " + name);
            emit LoafModified();
        }
    }

    void LoafEditorWidget::OnRemoveItem ( ) {
        QListWidgetItem *item = item_list_->currentItem();
        if (!item) {
            return;
        }

        QString item_text = item->text();
        QStringList parts = item_text.split(":");
        if (parts.isEmpty()) {
            return;
        }

        QString id = parts[0].trimmed();

        auto loaf = editor_->GetCurrentLoaf();
        if (!loaf) {
            return;
        }

        auto loaf_item = loaf->GetItem(id.toStdString());
        if (!loaf_item) {
            return;
        }

        bool has_configs = false;
        std::string depends_on = loaf_item->GetMetadata("depends_on");
        std::string args = loaf_item->GetMetadata("args");
        std::string working_dir = loaf_item->GetMetadata("working_dir");
        std::string auto_start = loaf_item->GetMetadata("auto_start");

        if (!depends_on.empty() || !args.empty() || !working_dir.empty() || auto_start == "true") {
            has_configs = true;
        }

        QString message = "Are you sure you want to remove '" + QString::fromStdString(loaf_item->GetName()) + "'?";
        if (has_configs) {
            message += "\n\nThis item has custom configurations that will be lost.";
        }

        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Confirm Remove",
            message,
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            if (editor_->RemoveItem(id.toStdString())) {
                RefreshItemList();
                status_label_->setText("Removed item: " + id);
                emit LoafModified();
            }
        }
    }

    void LoafEditorWidget::OnItemSelected ( ) {
        bool hasSelection = item_list_->currentItem() != nullptr;
        configure_button_->setEnabled(hasSelection);
    }

    void LoafEditorWidget::OnConfigureItem ( ) {
        QListWidgetItem *item = item_list_->currentItem();
        if (!item) {
            return;
        }

        QString item_text = item->text();
        QStringList parts = item_text.split(":");
        if (parts.size() == 0) {
            return;
        }

        QString id = parts[0].trimmed();
        auto loaf = editor_->GetCurrentLoaf();
        if (!loaf) {
            return;
        }

        auto loaf_item = loaf->GetItem(id.toStdString());
        if (!loaf_item) {
            return;
        }

        QDialog dialog(this);
        dialog.setWindowTitle("Configure Item: " + QString::fromStdString(loaf_item->GetName()));
        dialog.setMinimumWidth(500);

        QVBoxLayout *layout = new QVBoxLayout(&dialog);

        QGroupBox *deps_group = new QGroupBox("Dependencies", &dialog);
        QVBoxLayout *deps_layout = new QVBoxLayout(deps_group);

        QLabel *deps_label = new QLabel("Run after these items:", &dialog);
        deps_layout->addWidget(deps_label);

        QListWidget *deps_list = new QListWidget(&dialog);
        deps_list->setSelectionMode(QAbstractItemView::MultiSelection);

        const auto& all_items = loaf->GetItems();
        for (const auto& other_item : all_items) {
            if (other_item->GetId() != loaf_item->GetId()) {
                QString itemName = QString::fromStdString(other_item->GetName());
                deps_list->addItem(itemName);

                std::string dep_meta = loaf_item->GetMetadata("depends_on");
                if (!dep_meta.empty()) {
                    std::istringstream ss(dep_meta);
                    std::string token;
                    while (std::getline(ss, token, ',')) {
                        if (token == other_item->GetId()) {
                            deps_list->item(deps_list->count() - 1)->setSelected(true);
                            break;
                        }
                    }
                }
            }
        }

        deps_layout->addWidget(deps_list);
        layout->addWidget(deps_group);

        QGroupBox *config_group = new QGroupBox("Custom Configuration", &dialog);
        QVBoxLayout *config_layout = new QVBoxLayout(config_group);

        QLabel *args_label = new QLabel("Command-line Arguments:", &dialog);
        QLineEdit *args_edit = new QLineEdit(&dialog);
        args_edit->setText(QString::fromStdString(loaf_item->GetMetadata("args")));
        args_edit->setPlaceholderText("e.g., --verbose --config=myfile.conf");
        config_layout->addWidget(args_label);
        config_layout->addWidget(args_edit);

        QLabel *wd_label = new QLabel("Working Directory:", &dialog);
        QLineEdit *wd_edit = new QLineEdit(&dialog);
        wd_edit->setText(QString::fromStdString(loaf_item->GetMetadata("working_dir")));
        wd_edit->setPlaceholderText("Leave empty for default");

        QPushButton *wd_browse = new QPushButton("Browse...", &dialog);
        connect(wd_browse, &QPushButton::clicked, [&]() {
            QString dir = QFileDialog::getExistingDirectory(&dialog, "Select Working Directory");
            if (!dir.isEmpty()) {
                wd_edit->setText(dir);
            }
        });

        QHBoxLayout *wd_layout = new QHBoxLayout();
        wd_layout->addWidget(wd_edit);
        wd_layout->addWidget(wd_browse);
        config_layout->addWidget(wd_label);
        config_layout->addLayout(wd_layout);

        QCheckBox *auto_start_check = new QCheckBox("Auto-start when loaf runs", &dialog);
        auto_start_check->setChecked(loaf_item->GetMetadata("auto_start") == "true");
        config_layout->addWidget(auto_start_check);

        layout->addWidget(config_group);

        QDialogButtonBox *buttons = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        layout->addWidget(buttons);

        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted) {
            QStringList selectedDeps;
            for (int i = 0; i < deps_list->count(); ++i) {
                if (deps_list->item(i)->isSelected()) {
                    QString selected_name = deps_list->item(i)->text();
                    for (const auto& other_item : all_items) {
                        if (QString::fromStdString(other_item->GetName()) == selected_name) {
                            selectedDeps.append(QString::fromStdString(other_item->GetId()));
                            break;
                        }
                    }
                }
            }
            loaf_item->SetMetadata("depends_on", selectedDeps.join(",").toStdString());

            loaf_item->SetMetadata("args", args_edit->text().toStdString());
            loaf_item->SetMetadata("working_dir", wd_edit->text().toStdString());
            loaf_item->SetMetadata("auto_start", auto_start_check->isChecked() ? "true" : "false");

            status_label_->setText("Updated configuration for: " + QString::fromStdString(loaf_item->GetName()));
            emit LoafModified();
        }
    }

    void LoafEditorWidget::OnNameChanged ( ) {
        if (editor_->GetCurrentLoaf()) {
            editor_->SetLoafName(name_edit_->text().toStdString());
            emit LoafModified();
        }
    }

    void LoafEditorWidget::OnDescriptionChanged ( )  {
        if (editor_->GetCurrentLoaf()) {
            editor_->SetLoafDescription(description_edit_->toPlainText().toStdString());
            emit LoafModified();
        }
    }

    void LoafEditorWidget::UpdateLoafInfo ( ) {
        auto loaf = editor_->GetCurrentLoaf();
        if (loaf) {
            name_edit_->setText(QString::fromStdString(loaf->GetName()));
            description_edit_->setText(QString::fromStdString(loaf->GetDescription()));
        } else {
            name_edit_->clear();
            description_edit_->clear();
        }
    }

    void LoafEditorWidget::RefreshItemList ( ) {
        item_list_->clear();

        auto loaf = editor_->GetCurrentLoaf();
        if (loaf) {
            const auto& items = loaf->GetItems();
            for (const auto& item : items) {
                QString item_text = QString::fromStdString(
                    item->GetId() + ": " + item->GetName() + " - " + item->GetPath()
                );
                item_list_->addItem(item_text);
            }
        }
    }

    } // namespace GUI
} // namespace BreadBin
