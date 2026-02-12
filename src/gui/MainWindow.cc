#include "gui/MainWindow.h"
#include "gui/HomeWidget.h"
#include "gui/LoafEditorWidget.h"
#include "gui/TextEditorWidget.h"
#include "gui/ThemeEditorWidget.h"
#include "gui/LoafRuntimeWidget.h"
#include "gui/AppBrowserWidget.h"
#include "gui/LoafBrowserWidget.h"
#include "gui/ThemeBrowserWidget.h"
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QColor>
#include <QRegularExpression>
#include <QCloseEvent>
#include <fstream>
#include <sstream>

namespace BreadBin {
    namespace GUI {

        namespace {
            QString ParseThemeColourString(const std::string& raw_value, const QString& fallback) {
                const QString trimmed = QString::fromStdString(raw_value).trimmed();
                QColor direct(trimmed);
                if (direct.isValid()) {
                    return direct.name();
                }

                std::istringstream ss(raw_value);
                int red, green, blue, alpha = 255;
                if (ss >> red >> green >> blue) {
                    if (!(ss >> alpha)) {
                        alpha = 255;
                    }
                    return QColor(red, green, blue, alpha).name(QColor::HexArgb);
                }

                return fallback;
            }
        }

        MainWindow::MainWindow (QWidget *parent)
            : QMainWindow(parent),
              tab_widget_(nullptr),
              home_widget_(nullptr),
              loaf_editor_(nullptr),
              text_editor_(nullptr),
              theme_editor_(nullptr),
              runtime_widget_(nullptr),
              app_browser_(nullptr),
              loaf_browser_(nullptr),
              theme_browser_(nullptr)
        {
            setWindowTitle("Bread Bin - Loaf Management System");
            setMinimumSize(800, 600);
            resize(1200, 800);

            CreateCentralWidget();
            CreateMenus();
            CreateToolBar();
            ApplyWarmTheme();

            std::ifstream settings(GetThemeSettingsFile().toStdString());
            if (settings.is_open()) {
                std::string saved_theme_path;
                std::getline(settings, saved_theme_path);
                if (!saved_theme_path.empty() && QFile::exists(QString::fromStdString(saved_theme_path))) {
                    ApplyThemeFromFile(QString::fromStdString(saved_theme_path), false);
                }
            }

            statusBar()->showMessage("Ready - default loaf folder: " + GetDefaultLoafDirectory(), 8000);
        }

        MainWindow::~MainWindow ( ) {

        }

        void MainWindow::closeEvent (QCloseEvent *event) {
            if (CheckUnsavedChanges()) {
                event->accept();
            }
            else {
                event->ignore();
            }
        }

        void MainWindow::CreateCentralWidget ( ) {
            tab_widget_ = new QTabWidget(this);
            tab_widget_->setTabPosition(QTabWidget::North);
            tab_widget_->setMovable(true);

            home_widget_ = new HomeWidget(this);
            loaf_editor_ = new LoafEditorWidget(this);
            text_editor_ = new TextEditorWidget(this);
            theme_editor_ = new ThemeEditorWidget(this);
            runtime_widget_ = new LoafRuntimeWidget(this);
            app_browser_ = new AppBrowserWidget(this);
            loaf_browser_ = new LoafBrowserWidget(this);
            theme_browser_ = new ThemeBrowserWidget(this);

            tab_widget_->addTab(home_widget_, "🏠 Home");
            tab_widget_->addTab(loaf_editor_, "🍞 Loaf Editor");
            tab_widget_->addTab(text_editor_, "📝 Text Editor");
            tab_widget_->addTab(theme_editor_, "🎨 Theme Editor");
            tab_widget_->addTab(runtime_widget_, "▶️ Runtime");
            tab_widget_->addTab(app_browser_, "📱 App Browser");
            tab_widget_->addTab(loaf_browser_, "📁 Loaf Files");
            tab_widget_->addTab(theme_browser_, "🎨 Themes");

            connect(home_widget_, &HomeWidget::OpenLoafRequested,
                    this, &MainWindow::OnOpenLoafFromBrowser);
            connect(home_widget_, &HomeWidget::OpenLoafInTextEditorRequested,
                    this, &MainWindow::OnOpenLoafInTextEditor);
            connect(home_widget_, &HomeWidget::NewLoafRequested,
                    this, &MainWindow::NewLoaf);

            connect(app_browser_, &AppBrowserWidget::AddApplicationRequested,
                    this, &MainWindow::OnAddApplicationFromBrowser);
            connect(loaf_browser_, &LoafBrowserWidget::LoafOpened,
                    this, &MainWindow::OnOpenLoafFromBrowser);
            connect(theme_browser_, &ThemeBrowserWidget::ThemeApplied,
                    this, &MainWindow::OnApplyThemeFromBrowser);
            connect(theme_editor_, &ThemeEditorWidget::ThemeApplied,
                    this, &MainWindow::OnApplyThemeFromBrowser);
            connect(loaf_editor_, &LoafEditorWidget::LoafModified, this, [this]() {
                runtime_widget_->SetLoaf(loaf_editor_->GetCurrentLoaf());
            });
            connect(loaf_editor_, &LoafEditorWidget::CreateNewScriptRequested, this, [this]() {
                tab_widget_->setCurrentWidget(text_editor_);
                text_editor_->NewFile();
                statusBar()->showMessage("Create your script in the text editor, then save it", 5000);
            });

            setCentralWidget(tab_widget_);
        }

        void MainWindow::CreateMenus ( ) {
            file_menu_ = menuBar()->addMenu("&File");

            QAction *new_action = new QAction("&New Loaf", this);
            new_action->setShortcut(QKeySequence::New);
            connect(new_action, &QAction::triggered, this, &MainWindow::NewLoaf);
            file_menu_->addAction(new_action);

            QAction *open_action = new QAction("&Open Loaf...", this);
            open_action->setShortcut(QKeySequence::Open);
            connect(open_action, &QAction::triggered, this, &MainWindow::OpenLoaf);
            file_menu_->addAction(open_action);

            file_menu_->addSeparator();

            QAction *save_action = new QAction("&Save", this);
            save_action->setShortcut(QKeySequence::Save);
            connect(save_action, &QAction::triggered, this, &MainWindow::SaveLoaf);
            file_menu_->addAction(save_action);

            QAction *save_as_action = new QAction("Save &As...", this);
            save_as_action->setShortcut(QKeySequence::SaveAs);
            connect(save_as_action, &QAction::triggered, this, &MainWindow::SaveLoafAs);
            file_menu_->addAction(save_as_action);

            file_menu_->addSeparator();

            QAction *exit_action = new QAction("E&xit", this);
            exit_action->setShortcut(QKeySequence::Quit);
            connect(exit_action, &QAction::triggered, this, &QWidget::close);
            file_menu_->addAction(exit_action);

            edit_menu_ = menuBar()->addMenu("&Edit");

            view_menu_ = menuBar()->addMenu("&View");
            QAction *themeAction = new QAction("Apply &Theme", this);
            connect(themeAction, &QAction::triggered, this, &MainWindow::ApplyTheme);
            view_menu_->addAction(themeAction);

            help_menu_ = menuBar()->addMenu("&Help");
            QAction *aboutAction = new QAction("&About Bread Bin", this);
            connect(aboutAction, &QAction::triggered, this, &MainWindow::ShowAbout);
            help_menu_->addAction(aboutAction);
        }

        void MainWindow::CreateToolBar ( ) {
            main_tool_bar_ = addToolBar("Main Toolbar");
            main_tool_bar_->setMovable(false);

            QAction *new_tool_action = new QAction("New", this);
            connect(new_tool_action, &QAction::triggered, this, &MainWindow::NewLoaf);
            main_tool_bar_->addAction(new_tool_action);

            QAction *open_tool_action = new QAction("Open", this);
            connect(open_tool_action, &QAction::triggered, this, &MainWindow::OpenLoaf);
            main_tool_bar_->addAction(open_tool_action);

            QAction *save_tool_action = new QAction("Save", this);
            connect(save_tool_action, &QAction::triggered, this, &MainWindow::SaveLoaf);
            main_tool_bar_->addAction(save_tool_action);
        }

        void MainWindow::ApplyWarmTheme ( ) {
            QString style_sheet = R"(
                /* Main Window - Modern gradient background */
                QMainWindow {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                              stop:0 #faf7f2, stop:1 #f5f0e8);
                }

                /* Tab Widget - Modern card-style with shadow effect */
                QTabWidget::pane {
                    border: 1px solid #e0d5c7;
                    background-color: #ffffff;
                    border-radius: 8px;
                    padding: 4px;
                }
                QTabWidget::tab-bar {
                    left: 12px;
                }

                /* Tabs - Modern pill-style design */
                QTabBar::tab {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                              stop:0 #f5ede0, stop:1 #e8dcc8);
                    color: #5d4e37;
                    padding: 10px 20px;
                    margin-right: 4px;
                    margin-top: 4px;
                    border-top-left-radius: 8px;
                    border-top-right-radius: 8px;
                    font-weight: 600;
                    font-size: 13px;
                    min-width: 100px;
                }
                QTabBar::tab:selected {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                              stop:0 #e5b989, stop:1 #d4a574);
                    color: #2d1f0f;
                    margin-top: 2px;
                    padding: 12px 20px;
                }
                QTabBar::tab:hover:!selected {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                              stop:0 #ede1ce, stop:1 #ddc9a3);
                }

                /* Buttons - Modern flat design with smooth transitions */
                QPushButton {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                              stop:0 #e5b989, stop:1 #d4a574);
                    color: #2d1f0f;
                    border: none;
                    padding: 8px 16px;
                    border-radius: 6px;
                    font-weight: 600;
                    font-size: 13px;
                    min-height: 32px;
                }
                QPushButton:hover {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                              stop:0 #d4a574, stop:1 #c29860);
                }
                QPushButton:pressed {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                              stop:0 #c29860, stop:1 #b08850);
                    padding: 9px 15px 7px 17px;
                }
                QPushButton:disabled {
                    background-color: #e8dcc8;
                    color: #a89580;
                }

                /* Input Fields - Modern clean design */
                QLineEdit, QTextEdit, QPlainTextEdit {
                    background-color: #ffffff;
                    border: 2px solid #e8dcc8;
                    border-radius: 6px;
                    padding: 8px 12px;
                    color: #2d1f0f;
                    font-size: 13px;
                    selection-background-color: #d4a574;
                }
                QLineEdit:focus, QTextEdit:focus, QPlainTextEdit:focus {
                    border: 2px solid #d4a574;
                    background-color: #fffefb;
                }
                QLineEdit:hover, QTextEdit:hover, QPlainTextEdit:hover {
                    border: 2px solid #ddc9a3;
                }

                /* List Widgets - Modern card design */
                QListWidget {
                    background-color: #ffffff;
                    border: 2px solid #e8dcc8;
                    border-radius: 6px;
                    padding: 4px;
                    color: #2d1f0f;
                    font-size: 13px;
                    outline: none;
                }
                QListWidget:focus {
                    border: 2px solid #d4a574;
                }
                QListWidget::item {
                    padding: 8px;
                    border-radius: 4px;
                    margin: 2px;
                }
                QListWidget::item:selected {
                    background-color: #d4a574;
                    color: #2d1f0f;
                }
                QListWidget::item:hover:!selected {
                    background-color: #f5ede0;
                }

                /* Combo Box - Modern dropdown */
                QComboBox {
                    background-color: #ffffff;
                    border: 2px solid #e8dcc8;
                    border-radius: 6px;
                    padding: 6px 12px;
                    color: #2d1f0f;
                    font-size: 13px;
                    min-height: 28px;
                }
                QComboBox:hover {
                    border: 2px solid #ddc9a3;
                }
                QComboBox:focus {
                    border: 2px solid #d4a574;
                }
                QComboBox::drop-down {
                    border: none;
                    padding-right: 8px;
                }

                /* Group Boxes - Modern card style */
                QGroupBox {
                    background-color: #ffffff;
                    border: 2px solid #e8dcc8;
                    border-radius: 8px;
                    margin-top: 12px;
                    padding: 16px;
                    font-weight: 600;
                    font-size: 14px;
                    color: #5d4e37;
                }
                QGroupBox::title {
                    subcontrol-origin: margin;
                    subcontrol-position: top left;
                    left: 12px;
                    top: -8px;
                    background-color: #ffffff;
                    padding: 4px 8px;
                }

                /* Labels - Modern typography */
                QLabel {
                    color: #5d4e37;
                    font-size: 13px;
                }

                /* Menu Bar - Modern flat design */
                QMenuBar {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                              stop:0 #f5ede0, stop:1 #e8dcc8);
                    color: #5d4e37;
                    border-bottom: 1px solid #d4a574;
                    padding: 4px;
                    font-size: 13px;
                }
                QMenuBar::item {
                    padding: 6px 12px;
                    border-radius: 4px;
                }
                QMenuBar::item:selected {
                    background-color: #d4a574;
                    color: #2d1f0f;
                }
                QMenuBar::item:pressed {
                    background-color: #c29860;
                }

                /* Menu - Modern dropdown */
                QMenu {
                    background-color: #ffffff;
                    border: 1px solid #d4a574;
                    border-radius: 6px;
                    padding: 4px;
                }
                QMenu::item {
                    padding: 8px 24px;
                    border-radius: 4px;
                    font-size: 13px;
                }
                QMenu::item:selected {
                    background-color: #d4a574;
                    color: #2d1f0f;
                }
                QMenu::separator {
                    height: 1px;
                    background-color: #e8dcc8;
                    margin: 4px 8px;
                }

                /* Toolbar - Modern flat design */
                QToolBar {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                              stop:0 #f5ede0, stop:1 #e8dcc8);
                    border-bottom: 1px solid #d4a574;
                    spacing: 6px;
                    padding: 6px;
                }
                QToolButton {
                    background-color: transparent;
                    border: none;
                    padding: 6px 12px;
                    border-radius: 4px;
                    font-weight: 600;
                    color: #5d4e37;
                }
                QToolButton:hover {
                    background-color: #ddc9a3;
                }
                QToolButton:pressed {
                    background-color: #d4a574;
                }

                /* Status Bar - Modern info bar */
                QStatusBar {
                    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                              stop:0 #e8dcc8, stop:1 #ddc9a3);
                    color: #5d4e37;
                    border-top: 1px solid #d4a574;
                    font-size: 12px;
                    padding: 4px;
                }

                /* Scrollbars - Modern slim design */
                QScrollBar:vertical {
                    background-color: #f5f0e8;
                    width: 12px;
                    border-radius: 6px;
                }
                QScrollBar::handle:vertical {
                    background-color: #d4a574;
                    border-radius: 6px;
                    min-height: 30px;
                }
                QScrollBar::handle:vertical:hover {
                    background-color: #c29860;
                }
                QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                    height: 0px;
                }
                QScrollBar:horizontal {
                    background-color: #f5f0e8;
                    height: 12px;
                    border-radius: 6px;
                }
                QScrollBar::handle:horizontal {
                    background-color: #d4a574;
                    border-radius: 6px;
                    min-width: 30px;
                }
                QScrollBar::handle:horizontal:hover {
                    background-color: #c29860;
                }
                QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
                    width: 0px;
                }

                /* Check Boxes - Modern toggle style */
                QCheckBox {
                    spacing: 8px;
                    color: #5d4e37;
                    font-size: 13px;
                }
                QCheckBox::indicator {
                    width: 20px;
                    height: 20px;
                    border-radius: 4px;
                    border: 2px solid #d4a574;
                    background-color: #ffffff;
                }
                QCheckBox::indicator:checked {
                    background-color: #d4a574;
                    border: 2px solid #d4a574;
                }
                QCheckBox::indicator:hover {
                    border: 2px solid #c29860;
                }
            )";

            setStyleSheet(style_sheet);
        }

        QString MainWindow::GetThemeSettingsFile ( ) const {
            const QString config_dir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
            QDir().mkpath(config_dir);
            return config_dir + "/theme.conf";
        }

        bool MainWindow::CheckUnsavedChanges ( ) {
            if (loaf_editor_->HasUnsavedChanges()) {
                QMessageBox::StandardButton reply = QMessageBox::question(
                    this,
                    "Unsaved Changes",
                    "You have unsaved changes. What would you like to do?",
                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
                );

                if (reply == QMessageBox::Save) {
                    SaveLoaf();
                    return true;
                }
                else {
                    if (reply == QMessageBox::Discard) {
                        return true;
                    }
                    else {
                        return false;
                    }
                }
            }
            return true;
        }

        void MainWindow::NewLoaf ( ) {
            if (!CheckUnsavedChanges()) {
                return;
            }

            loaf_editor_->NewLoaf();
            runtime_widget_->SetLoaf(loaf_editor_->GetCurrentLoaf());
            tab_widget_->setCurrentWidget(loaf_editor_);
            statusBar()->showMessage("Created new loaf", 3000);
        }

        QString MainWindow::GetDefaultLoafDirectory ( ) const {
            QString default_dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/BreadBin/loafs";

            QDir dir(default_dir);
            if (!dir.exists()) {
                dir.mkpath(".");
            }

            return default_dir;
        }

        void MainWindow::OpenLoaf ( ) {
            if (!CheckUnsavedChanges()) {
                return;
            }

            QString filepath = QFileDialog::getOpenFileName(
                this,
                "Open Loaf",
                GetDefaultLoafDirectory(),
                "Loaf Files (*.loaf);;All Files (*)"
            );

            if (!filepath.isEmpty()) {
                OpenLoafFile(filepath);
            }
        }

        bool MainWindow::OpenLoafFile (const QString& filepath) {
            if (loaf_editor_->OpenLoaf(filepath)) {
                runtime_widget_->SetLoaf(loaf_editor_->GetCurrentLoaf());
                loaf_browser_->RefreshLoafFiles();

                auto loaf = loaf_editor_->GetCurrentLoaf();
                QString loaf_name = loaf ? QString::fromStdString(loaf->GetName()) : "";

                home_widget_->AddRecentLoaf(filepath, loaf_name);

                tab_widget_->setCurrentWidget(loaf_editor_);
                statusBar()->showMessage("Opened loaf: " + filepath, 3000);
                return true;
            }
            else {
                QMessageBox::warning(this, "Error", "Failed to open loaf file");
                return false;
            }
        }

        void MainWindow::SaveLoaf ( ) {
            QString current_path = loaf_editor_->GetCurrentFilePath();

            if (!current_path.isEmpty()) {
                if (loaf_editor_->SaveLoaf()) {
                    loaf_browser_->RefreshLoafFiles();
                    statusBar()->showMessage("Saved loaf: " + current_path, 3000);
                }
                else {
                    QMessageBox::warning(this, "Error", "Failed to save loaf file");
                }
            }
            else {
                SaveLoafAs();
            }
        }

        void MainWindow::SaveLoafAs ( ) {
            QString default_path = GetDefaultLoafDirectory() + "/my_loaf.loaf";

            auto loaf = loaf_editor_->GetCurrentLoaf();
            if (loaf) {
                QString loaf_name = QString::fromStdString(loaf->GetName());
                if (!loaf_name.isEmpty()) {
                    loaf_name.replace(QRegularExpression("[^a-zA-Z0-9_\\-]"), "_");
                    default_path = GetDefaultLoafDirectory() + "/" + loaf_name + ".loaf";
                }
            }

            QString filepath = QFileDialog::getSaveFileName(
                this,
                "Save Loaf As",
                default_path,
                "Loaf Files (*.loaf);;All Files (*)"
            );

            if (!filepath.isEmpty()) {
                if (!filepath.endsWith(".loaf")) {
                    filepath += ".loaf";
                }

                if (loaf_editor_->SaveLoaf(filepath)) {
                    loaf_browser_->RefreshLoafFiles();

                    auto loaf = loaf_editor_->GetCurrentLoaf();
                    QString loaf_name = loaf ? QString::fromStdString(loaf->GetName()) : "";
                    home_widget_->AddRecentLoaf(filepath, loaf_name);

                    statusBar()->showMessage("Saved loaf: " + filepath, 3000);
                }
                else {
                    QMessageBox::warning(this, "Error", "Failed to save loaf file");
                }
            }
        }

        void MainWindow::ShowAbout ( ) {
            QMessageBox::about(
                this,
                "About Bread Bin",
                "<h2>Bread Bin</h2>"
                "<p><b>Version 0.1</b></p>"
                "<p>A cross-platform application for organizing, managing, and controlling loafs — "
                "self-contained libraries of applications, files, scripts, and configuration.</p>"
                "<p>Designed for Linux, Windows, and Android with a warm, minimal interface.</p>"
            );
        }

        void MainWindow::ApplyTheme ( ) {
            if (!current_theme_path_.isEmpty() && QFile::exists(current_theme_path_)) {
                ApplyThemeFromFile(current_theme_path_, false);
                statusBar()->showMessage("Re-applied theme", 3000);
                return;
            }

            ApplyWarmTheme();
            statusBar()->showMessage("Applied built-in warm theme", 3000);
        }

        void MainWindow::OnAddApplicationFromBrowser (const AppInfo& app_info) {
            tab_widget_->setCurrentWidget(loaf_editor_);

            statusBar()->showMessage("Added application: " + QString::fromStdString(app_info.name), 3000);
        }

        void MainWindow::OnOpenLoafFromBrowser (const QString& filepath) {
            if (!CheckUnsavedChanges()) {
                return;
            }

            OpenLoafFile(filepath);
        }

        void MainWindow::OnOpenLoafInTextEditor (const QString& filepath) {
            text_editor_->OpenFile(filepath);
            tab_widget_->setCurrentWidget(text_editor_);
            statusBar()->showMessage("Opened in text editor: " + filepath, 3000);
        }

        void MainWindow::OnApplyThemeFromBrowser (const QString& filepath) {
            ApplyThemeFromFile(filepath, true);
        }

        void MainWindow::ApplyThemeFromFile (const QString& filepath, bool persist_selection) {
            std::ifstream file(filepath.toStdString());
            if (!file.is_open()) {
                QMessageBox::warning(this, "Error", "Failed to load theme file");
                ApplyWarmTheme();
                return;
            }

            QString primary_colour = "#d4a574";
            QString secondary_colour = "#f5f0e8";
            QString background_colour = "#faf7f2";
            QString text_colour = "#2d1f0f";

            std::string line;
            while (std::getline(file, line)) {
                if (line.find("PRIMARY_COLOUR:") == 0) {
                    primary_colour = ParseThemeColourString(line.substr(14), primary_colour);
                }
                else {
                    if (line.find("SECONDARY_COLOUR:") == 0) {
                        secondary_colour = ParseThemeColourString(line.substr(16), secondary_colour);
                    }
                    else {
                        if (line.find("BACKGROUND_COLOUR:") == 0) {
                            background_colour = ParseThemeColourString(line.substr(17), background_colour);
                        }
                        else {
                            if (line.find("TEXT_COLOUR:") == 0) {
                                text_colour = ParseThemeColourString(line.substr(11), text_colour);
                            }
                        }
                    }
                }
            }

            QString style_sheet = QString(R"(
                QMainWindow {
                    background-color: %1;
                }
                QTabWidget::pane {
                    border: 2px solid %2;
                    background-color: %1;
                    border-radius: 4px;
                }
                QTabWidget::tab-bar {
                    left: 8px;
                }
                QTabBar::tab {
                    background-color: %3;
                    color: %4;
                    padding: 8px 16px;
                    margin-right: 2px;
                    border-top-left-radius: 4px;
                    border-top-right-radius: 4px;
                    font-weight: bold;
                }
                QTabBar::tab:selected {
                    background-color: %2;
                    color: %4;
                }
                QTabBar::tab:hover {
                    background-color: %3;
                }
                QPushButton {
                    background-color: %2;
                    color: %4;
                    border: none;
                    padding: 6px 12px;
                    border-radius: 4px;
                    font-weight: bold;
                }
                QPushButton:hover {
                    background-color: %2;
                }
                QPushButton:pressed {
                    background-color: %2;
                }
                QLineEdit, QTextEdit, QPlainTextEdit {
                    background-color: #ffffff;
                    border: 1px solid %2;
                    border-radius: 3px;
                    padding: 4px;
                    color: %4;
                }
                QListWidget {
                    background-color: #ffffff;
                    border: 1px solid %2;
                    border-radius: 3px;
                    color: %4;
                }
                QLabel {
                    color: %4;
                }
                QMenuBar {
                    background-color: %3;
                    color: %4;
                }
                QMenuBar::item:selected {
                    background-color: %2;
                }
                QMenu {
                    background-color: %1;
                    border: 1px solid %2;
                }
                QMenu::item:selected {
                    background-color: %2;
                }
                QToolBar {
                    background-color: %3;
                    border: none;
                    spacing: 4px;
                }
                QStatusBar {
                    background-color: %3;
                    color: %4;
                }
            )").arg(background_colour, primary_colour, secondary_colour, text_colour);

            setStyleSheet(style_sheet);
            current_theme_path_ = filepath;

            if (persist_selection) {
                std::ofstream settings(GetThemeSettingsFile().toStdString());
                if (settings.is_open()) {
                    settings << filepath.toStdString() << "\n";
                }
            }

            statusBar()->showMessage("Applied theme from: " + filepath, 3000);
        }

    } // namespace GUI
} // namespace BreadBin
