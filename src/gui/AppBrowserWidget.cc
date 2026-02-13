#include "gui/AppBrowserWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QProgressDialog>
#include <QFileInfo>
#include <QDir>
#include <QLabel>

namespace BreadBin {
    namespace GUI {

    AppBrowserWidget::AppBrowserWidget (QWidget *parent)
        : QWidget(parent),
          discovery_(std::make_shared<AppDiscovery>())
    {
        SetupUI();
        ConnectSignals();

        QString cache_file = QDir::homePath() + "/.breadbin_app_cache";
        if (QFileInfo::exists(cache_file)) {
            discovery_->LoadCache(cache_file.toStdString());
            UpdateApplicationList();
        }
    }

    AppBrowserWidget::~AppBrowserWidget ( ) {

    }

    void AppBrowserWidget::SetupUI ( ) {
        QVBoxLayout *main_layout = new QVBoxLayout(this);
        main_layout->setContentsMargins(16, 16, 16, 16);
        main_layout->setSpacing(16);

        QGroupBox *filter_group = new QGroupBox("🔍 Search & Filter", this);
        QVBoxLayout *filter_layout = new QVBoxLayout(filter_group);
        filter_layout->setContentsMargins(12, 20, 12, 12);
        filter_layout->setSpacing(12);

        QHBoxLayout *search_layout = new QHBoxLayout();
        search_layout->setSpacing(12);
        QLabel *search_label = new QLabel("Search:", this);
        search_label->setStyleSheet("font-weight: 600; font-size: 13px; min-width: 70px;");
        search_edit_ = new QLineEdit(this);
        search_edit_->setPlaceholderText("Search applications...");
        search_edit_->setMinimumHeight(36);
        search_layout->addWidget(search_label);
        search_layout->addWidget(search_edit_, 1);
        filter_layout->addLayout(search_layout);

        QHBoxLayout *category_layout = new QHBoxLayout();
        category_layout->setSpacing(12);
        QLabel *category_label = new QLabel("Category:", this);
        category_label->setStyleSheet("font-weight: 600; font-size: 13px; min-width: 70px;");
        category_combo_ = new QComboBox(this);
        category_combo_->addItem("All Categories");
        category_combo_->setMinimumHeight(36);
        category_layout->addWidget(category_label);
        category_layout->addWidget(category_combo_, 1);
        filter_layout->addLayout(category_layout);

        main_layout->addWidget(filter_group);

        QHBoxLayout *content_layout = new QHBoxLayout();
        content_layout->setSpacing(16);

        QGroupBox *list_group = new QGroupBox("📱 Available Applications", this);
        list_group->setMinimumWidth(300);
        QVBoxLayout *list_layout = new QVBoxLayout(list_group);
        list_layout->setContentsMargins(12, 20, 12, 12);
        list_layout->setSpacing(12);

        app_list_ = new QListWidget(this);
        app_list_->setMinimumHeight(250);
        list_layout->addWidget(app_list_, 1);

        QHBoxLayout *button_layout = new QHBoxLayout();
        button_layout->setSpacing(8);
        scan_button_ = new QPushButton("🔄 Scan System", this);
        scan_button_->setMinimumHeight(40);
        scan_button_->setMinimumWidth(130);
        add_button_ = new QPushButton("➕ Add to Loaf", this);
        add_button_->setMinimumHeight(40);
        add_button_->setMinimumWidth(130);
        add_button_->setEnabled(false);
        button_layout->addWidget(scan_button_);
        button_layout->addWidget(add_button_);
        button_layout->addStretch();
        list_layout->addLayout(button_layout);

        content_layout->addWidget(list_group, 1);

        QGroupBox *details_group = new QGroupBox("ℹ️ Application Details", this);
        details_group->setMinimumWidth(350);
        QVBoxLayout *details_layout = new QVBoxLayout(details_group);
        details_layout->setContentsMargins(12, 20, 12, 12);

        details_text_ = new QTextEdit(this);
        details_text_->setReadOnly(true);
        details_text_->setMinimumHeight(250);
        details_text_->setStyleSheet(R"(
            QTextEdit {
                background-color: #fafafa;
                border: 2px solid #e8dcc8;
                border-radius: 6px;
                padding: 12px;
                color: #2d1f0f;
                font-size: 13px;
            }
        )");
        details_layout->addWidget(details_text_, 1);

        content_layout->addWidget(details_group, 1);

        main_layout->addLayout(content_layout, 1);

        status_label_ = new QLabel("Ready. Click 'Scan System' to discover applications.", this);
        status_label_->setStyleSheet("color: #8b7a5e; font-size: 12px; padding: 8px;");
        main_layout->addWidget(status_label_);

        setLayout(main_layout);
    }

    void AppBrowserWidget::ConnectSignals ( ) {
        connect(scan_button_, &QPushButton::clicked, this, &AppBrowserWidget::OnScanClicked);
        connect(search_edit_, &QLineEdit::textChanged, this, &AppBrowserWidget::OnSearchChanged);
        connect(category_combo_, QOverload<const QString&>::of(&QComboBox::currentTextChanged),
                this, &AppBrowserWidget::OnCategoryChanged);
        connect(app_list_, &QListWidget::itemSelectionChanged, this, &AppBrowserWidget::OnApplicationSelected);
        connect(add_button_, &QPushButton::clicked, this, &AppBrowserWidget::OnAddToLoafClicked);
    }

    void AppBrowserWidget::OnScanClicked ( ) {
        QProgressDialog progress("Scanning system for applications...", "Cancel", 0, 0, this);
        progress.setWindowModality(Qt::WindowModal);
        progress.show();

        size_t count = discovery_->ScanSystem();

        progress.close();

        PopulateCategories();
        UpdateApplicationList();

        status_label_->setText(QString("Found %1 applications").arg(count));

        QString cache_file = QDir::homePath() + "/.breadbin_app_cache";
        discovery_->SaveCache(cache_file.toStdString());
    }

    void AppBrowserWidget::OnSearchChanged (const QString& text) {
        UpdateApplicationList();
    }

    void AppBrowserWidget::OnCategoryChanged (const QString& category) {
        UpdateApplicationList();
    }

    void AppBrowserWidget::OnApplicationSelected ( ) {
        QListWidgetItem *item = app_list_->currentItem();
        if (!item) {
            add_button_->setEnabled(false);
            details_text_->clear();
            return;
        }

        add_button_->setEnabled(true);

        int index = app_list_->currentRow();
        if (index >= 0 && index < filtered_apps_.size()) {
            UpdateApplicationDetails(filtered_apps_[index]);
            emit ApplicationSelected(filtered_apps_[index]);
        }
    }

    void AppBrowserWidget::OnAddToLoafClicked ( ) {
        const AppInfo* app = GetSelectedApplication();
        if (app) {
            emit AddApplicationRequested(*app);
            status_label_->setText("Application added: " + QString::fromStdString(app->name));
        }
    }

    void AppBrowserWidget::RefreshApplications ( ) {
        UpdateApplicationList();
    }

    const AppInfo* AppBrowserWidget::GetSelectedApplication ( ) const {
        int index = app_list_->currentRow();
        if (index >= 0 && index < filtered_apps_.size()) {
            return &filtered_apps_[index];
        }
        return nullptr;
    }

    void AppBrowserWidget::UpdateApplicationList ( ) {
        app_list_->clear();
        filtered_apps_.clear();

        const auto& all_apps = discovery_->GetApplications();

        QString search_text = search_edit_->text().toLower();
        QString category = category_combo_->currentText();

        for (const auto& app : all_apps) {
            if (!search_text.isEmpty()) {
                QString app_name = QString::fromStdString(app.name).toLower();
                if (!app_name.contains(search_text)) {
                    continue;
                }
            }

            if (category != "All Categories") {
                if (QString::fromStdString(app.category) != category) {
                    continue;
                }
            }

            filtered_apps_.push_back(app);
        }

        for (const auto& app : filtered_apps_) {
            QString display_text = QString::fromStdString(app.name);
            if (!app.category.empty()) {
                display_text += " [" + QString::fromStdString(app.category) + "]";
            }
            app_list_->addItem(display_text);
        }

        status_label_->setText(QString("Showing %1 of %2 applications")
                              .arg(filtered_apps_.size())
                              .arg(all_apps.size()));
    }

    void AppBrowserWidget::UpdateApplicationDetails (const AppInfo& app_info) {
        QString details;

        details += "<h3>" + QString::fromStdString(app_info.name) + "</h3>";

        if (!app_info.description.empty()) {
            details += "<p><b>Description:</b> " + QString::fromStdString(app_info.description) + "</p>";
        }

        details += "<p><b>Executable:</b> <code>" + QString::fromStdString(app_info.executable) + "</code></p>";

        if (!app_info.category.empty()) {
            details += "<p><b>Category:</b> " + QString::fromStdString(app_info.category) + "</p>";
        }

        if (!app_info.icon_path.empty()) {
            details += "<p><b>Icon:</b> " + QString::fromStdString(app_info.icon_path) + "</p>";
        }

        details += "<hr>";
        details += "<h4>Configuration Options</h4>";
        details += "<p><b>Common Command-Line Flags:</b></p>";
        details += "<ul>";
        details += "<li><code>--help</code> - Display help information</li>";
        details += "<li><code>--version</code> - Show version information</li>";
        details += "<li><code>--config &lt;file&gt;</code> - Specify config file</li>";
        details += "</ul>";

        details += "<p><b>Window Positioning:</b></p>";
        details += "<ul>";
        details += "<li>Position handling: Default (system-managed)</li>";
        details += "<li>Launch behavior: Normal window</li>";
        details += "<li>Workspace: Current workspace</li>";
        details += "</ul>";

        if (!app_info.metadata.empty()) {
            details += "<p><b>Additional Metadata:</b></p>";
            details += "<ul>";
            for (const auto& [key, value] : app_info.metadata) {
                details += "<li>" + QString::fromStdString(key) + ": " +
                          QString::fromStdString(value) + "</li>";
            }
            details += "</ul>";
        }

        details_text_->setHtml(details);
    }

    void AppBrowserWidget::PopulateCategories ( ) {
        QString current_category = category_combo_->currentText();
        category_combo_->clear();
        category_combo_->addItem("All Categories");

        auto categories = discovery_->GetCategories();
        for (const auto& category : categories) {
            category_combo_->addItem(QString::fromStdString(category));
        }

        int index = category_combo_->findText(current_category);
        if (index >= 0) {
            category_combo_->setCurrentIndex(index);
        }
    }

    } // namespace GUI
} // namespace BreadBin
