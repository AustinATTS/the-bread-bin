#include "gui/TextEditorWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QFileInfo>

namespace BreadBin {
    namespace GUI {

    TextEditorWidget::TextEditorWidget (QWidget *parent)
        : QWidget(parent)
    {
        SetupUI();
        ConnectSignals();
        CreateNewTab();
    }

    TextEditorWidget::~TextEditorWidget ( ) {

    }

    void TextEditorWidget::SetupUI ( ) {
        QVBoxLayout *main_layout = new QVBoxLayout(this);
        main_layout->setContentsMargins(16, 16, 16, 16);
        main_layout->setSpacing(12);

        QHBoxLayout *toolbar_layout = new QHBoxLayout();
        toolbar_layout->setSpacing(8);

        new_file_button_ = new QPushButton("📄 New", this);
        new_file_button_->setToolTip("Create a new file");
        new_file_button_->setMinimumWidth(80);

        open_file_button_ = new QPushButton("📂 Open", this);
        open_file_button_->setToolTip("Open an existing file");
        open_file_button_->setMinimumWidth(80);

        save_file_button_ = new QPushButton("💾 Save", this);
        save_file_button_->setToolTip("Save the current file");
        save_file_button_->setMinimumWidth(80);

        find_button_ = new QPushButton("🔍 Find", this);
        find_button_->setToolTip("Find text in the current file");
        find_button_->setMinimumWidth(80);

        replace_button_ = new QPushButton("🔄 Replace", this);
        replace_button_->setToolTip("Find and replace text");
        replace_button_->setMinimumWidth(80);

        toolbar_layout->addWidget(new_file_button_);
        toolbar_layout->addWidget(open_file_button_);
        toolbar_layout->addWidget(save_file_button_);
        toolbar_layout->addSpacing(12);
        toolbar_layout->addWidget(find_button_);
        toolbar_layout->addWidget(replace_button_);
        toolbar_layout->addStretch();

        main_layout->addLayout(toolbar_layout);

        tab_widget_ = new QTabWidget(this);
        tab_widget_->setTabsClosable(true);
        tab_widget_->setMovable(true);
        tab_widget_->setDocumentMode(true);
        main_layout->addWidget(tab_widget_, 1);

        status_label_ = new QLabel("Ready", this);
        status_label_->setStyleSheet("color: #8b7a5e; font-size: 12px; padding: 8px;");
        main_layout->addWidget(status_label_);

        setLayout(main_layout);
    }

    void TextEditorWidget::ConnectSignals ( ) {
        connect(new_file_button_, &QPushButton::clicked, this, &TextEditorWidget::OnNewFile);
        connect(open_file_button_, &QPushButton::clicked, this, &TextEditorWidget::OnOpenFile);
        connect(save_file_button_, &QPushButton::clicked, this, &TextEditorWidget::OnSaveFile);
        connect(find_button_, &QPushButton::clicked, this, &TextEditorWidget::OnFind);
        connect(replace_button_, &QPushButton::clicked, this, &TextEditorWidget::OnReplace);
        connect(tab_widget_, &QTabWidget::currentChanged, this, &TextEditorWidget::OnTabChanged);
        connect(tab_widget_, &QTabWidget::tabCloseRequested, this, &TextEditorWidget::OnCloseTab);
    }

    void TextEditorWidget::CreateNewTab (const QString &title) {
        QPlainTextEdit *editor = new QPlainTextEdit(this);

        QFont mono_font;
        mono_font.setPointSize(11);
        mono_font.setStyleHint(QFont::Monospace);
        mono_font.setFamilies({"Consolas", "Monaco", "Courier New", "monospace"});
        editor->setFont(mono_font);

        editor->setLineWrapMode(QPlainTextEdit::NoWrap);

        editor->setStyleSheet(R"(
            QPlainTextEdit {
                background-color: #ffffff;
                border: 2px solid #e8dcc8;
                border-radius: 6px;
                padding: 12px;
                color: #2d1f0f;
                font-size: 11pt;
                selection-background-color: #d4a574;
                line-height: 1.5;
            }
            QPlainTextEdit:focus {
                border: 2px solid #d4a574;
            }
        )");

        auto text_editor = std::make_shared<TextEditor> ( );
        editors_.push_back(text_editor);

        int index = tab_widget_->addTab(editor, title);
        tab_widget_->setCurrentIndex(index);

        file_paths_.append("");

        connect(editor, &QPlainTextEdit::textChanged, this, &TextEditorWidget::OnTextChanged);
    }

    void TextEditorWidget::NewFile ( ) {
        OnNewFile();
    }

    void TextEditorWidget::OnNewFile ( ) {
        CreateNewTab("Untitled");
        status_label_->setText("Created new file");
    }

    void TextEditorWidget::OnOpenFile ( ) {
        QString filepath = QFileDialog::getOpenFileName(
            this,
            "Open File",
            "",
            "All Files (*)"
        );

        if (!filepath.isEmpty()) {
            OpenFile(filepath);
        }
    }

    bool TextEditorWidget::OpenFile (const QString &filepath) {
        QFile file(filepath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Error", "Could not open file: " + filepath);
            return false;
        }

        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        int current_index = GetCurrentTabIndex();
        QPlainTextEdit *editor = GetCurrentEditor();

        if (editor && editor->toPlainText().isEmpty() && file_paths_[current_index].isEmpty()) {
            editor->setPlainText(content);
            file_paths_[current_index] = filepath;
            tab_widget_->setTabText(current_index, QFileInfo(filepath).fileName());
        }
        else {
            CreateNewTab(QFileInfo(filepath).fileName());
            editor = GetCurrentEditor();
            current_index = GetCurrentTabIndex();
            editor->setPlainText(content);
            file_paths_[current_index] = filepath;
        }

        if (current_index >= 0 && current_index < editors_.size()) {
            editors_[current_index]->OpenFile(filepath.toStdString());
        }

        status_label_->setText("Opened: " + filepath);
        return true;
    }

    void TextEditorWidget::OnSaveFile() {
        int current_index = GetCurrentTabIndex();
        if (current_index < 0) return;

        QString filepath = file_paths_[current_index];

        if (filepath.isEmpty()) {
            filepath = QFileDialog::getSaveFileName(
                this,
                "Save File",
                "",
                "All Files (*)"
            );

            if (filepath.isEmpty()) return;
            file_paths_[current_index] = filepath;
            tab_widget_->setTabText(current_index, QFileInfo(filepath).fileName());
        }

        SaveFile(filepath);
    }

    bool TextEditorWidget::SaveFile (const QString &filepath) {
        QPlainTextEdit *editor = GetCurrentEditor();
        if (!editor) return false;

        QFile file(filepath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, "Error", "Could not save file: " + filepath);
            return false;
        }

        QTextStream out(&file);
        out << editor->toPlainText();
        file.close();

        status_label_->setText("Saved: " + filepath);
        return true;
    }

    bool TextEditorWidget::SaveCurrentFile ( ) {
        int current_index = GetCurrentTabIndex();
        if (current_index < 0) return false;

        QString filepath = file_paths_[current_index];
        if (filepath.isEmpty()) {
            OnSaveFile();
            return true;
        }

        return SaveFile(filepath);
    }

    bool TextEditorWidget::HasUnsavedChanges ( ) const {
        for (int i = 0; i < tab_widget_->count(); ++i) {
            QString tab_text = tab_widget_->tabText(i);
            if (tab_text.endsWith("*")) {
                return true;
            }
        }
        return false;
    }

    void TextEditorWidget::OnFind ( ) {
        QPlainTextEdit *editor = GetCurrentEditor();
        if (!editor) return;

        bool ok;
        QString search_text = QInputDialog::getText(
            this,
            "Find",
            "Search for:",
            QLineEdit::Normal,
            "",
            &ok
        );

        if (ok && !search_text.isEmpty()) {
            if (editor->find(search_text)) {
                status_label_->setText("Found: " + search_text);
            }
            else {
                status_label_->setText("Not found: " + search_text);
            }
        }
    }

    void TextEditorWidget::OnReplace ( ) {
        QPlainTextEdit *editor = GetCurrentEditor();
        if (!editor) return;

        bool ok;
        QString search_text = QInputDialog::getText(
            this,
            "Replace",
            "Search for:",
            QLineEdit::Normal,
            "",
            &ok
        );

        if (!ok || search_text.isEmpty()) return;

        QString replace_text = QInputDialog::getText(
            this,
            "Replace",
            "Replace with:",
            QLineEdit::Normal,
            "",
            &ok
        );

        if (!ok) return;

        QString content = editor->toPlainText();
        int count = content.count(search_text);
        if (count > 0) {
            content.replace(search_text, replace_text);
            editor->setPlainText(content);
            status_label_->setText(QString("Replaced %1 occurrence(s)").arg(count));
        }
        else {
            status_label_->setText("No occurrences found");
        }
    }

    void TextEditorWidget::OnTextChanged ( ) {
        int currentIndex = GetCurrentTabIndex();
        if (currentIndex >= 0) {
            QString current_title = tab_widget_->tabText(currentIndex);
            if (!current_title.endsWith("*")) {
                tab_widget_->setTabText(currentIndex, current_title + "*");
            }
            emit FileModified();
        }
    }

    void TextEditorWidget::OnTabChanged (int index) {
        if (index >= 0 && index < file_paths_.size()) {
            if (!file_paths_[index].isEmpty()) {
                status_label_->setText("Editing: " + file_paths_[index]);
            }
            else {
                status_label_->setText("Editing: Untitled");
            }
        }
    }

    void TextEditorWidget::OnCloseTab (int index) {
        if (index >= 0 && index < tab_widget_->count()) {
            tab_widget_->removeTab(index);
            if (index < editors_.size()) {
                editors_.erase(editors_.begin() + index);
            }
            if (index < file_paths_.size()) {
                file_paths_.removeAt(index);
            }

            if (tab_widget_->count() == 0) {
                CreateNewTab();
            }
        }
    }

    int TextEditorWidget::GetCurrentTabIndex ( ) const {
        return tab_widget_->currentIndex();
    }

    QPlainTextEdit* TextEditorWidget::GetCurrentEditor ( ) const {
        int index = GetCurrentTabIndex();
        if (index >= 0) {
            return qobject_cast<QPlainTextEdit*>(tab_widget_->widget(index));
        }
        return nullptr;
    }

    } // namespace GUI
} // namespace BreadBin
