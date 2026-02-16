#include "Loaf.h"
#include <fstream>
#include <algorithm>

namespace BreadBin {

    namespace {
        std::shared_ptr<LoafItem> CreateItemByType(const std::string& type, const std::string& id) {
            if (type == "APPLICATION") {
                return std::make_shared<ApplicationItem>(id);
            }
            if (type == "FILE") {
                return std::make_shared<FileItem>(id);
            }
            if (type == "CONFIG") {
                return std::make_shared<ConfigItem>(id);
            }
            if (type == "SCRIPT") {
                return std::make_shared<ScriptItem>(id);
            }
            if (type == "WEBPAGE") {
                return std::make_shared<WebPageItem>(id);
            }
            return nullptr;
        }

        std::string ToTypeString(LoafItem::Type type) {
            switch (type) {
                case LoafItem::Type::APPLICATION:
                    return "APPLICATION";
                case LoafItem::Type::FILE:
                    return "FILE";
                case LoafItem::Type::CONFIG:
                    return "CONFIG";
                case LoafItem::Type::SCRIPT:
                    return "SCRIPT";
                case LoafItem::Type::WEBPAGE:
                    return "WEBPAGE";
                default:
                    return "UNKNOWN";
            }
        }
    }

    Loaf::Loaf ( ) : name_("Untitled Loaf"), description_(""), layout_("default"), running_(false) {

    }

    Loaf::Loaf (const std::string& name)
        : name_(name), description_(""), layout_("default"), running_(false) {

    }

    Loaf::~Loaf ( ) {
        if (running_) {
            Stop();
        }
    }

    bool Loaf::Load (const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        Clear();

        std::getline(file, name_);
        std::getline(file, description_);
        std::getline(file, layout_);

        int ruleCount = 0;
        file >> ruleCount;
        file.ignore();

        for (int i = 0; i < ruleCount; ++i) {
            std::string key, value;
            std::getline(file, key);
            std::getline(file, value);
            runtime_rules_[key] = value;
        }

        int itemCount = 0;
        if (!(file >> itemCount)) {
            return true;
        }
        file.ignore();

        for (int i = 0; i < itemCount; ++i) {
            std::string type_line;
            std::getline(file, type_line);
            if (type_line.rfind("TYPE:", 0) != 0) {
                continue;
            }

            std::string id_line;
            std::getline(file, id_line);
            if (id_line.rfind("ID:", 0) != 0) {
                continue;
            }

            std::string name_line;
            std::getline(file, name_line);
            std::string path_line;
            std::getline(file, path_line);

            const std::string type = type_line.substr(5);
            const std::string id = id_line.substr(3);
            auto item = CreateItemByType(type, id);
            if (!item) {
                continue;
            }

            if (name_line.rfind("NAME:", 0) == 0) {
                item->SetName(name_line.substr(5));
            }
            if (path_line.rfind("PATH:", 0) == 0) {
                item->SetPath(path_line.substr(5));
            }

            std::string meta_count_line;
            std::getline(file, meta_count_line);
            int meta_count = 0;
            if (meta_count_line.rfind("META_COUNT:", 0) == 0) {
                meta_count = std::stoi(meta_count_line.substr(11));
            }

            for (int m = 0; m < meta_count; ++m) {
                std::string key_line;
                std::string value_line;
                std::getline(file, key_line);
                std::getline(file, value_line);

                if (key_line.rfind("META_KEY:", 0) == 0 && value_line.rfind("META_VALUE:", 0) == 0) {
                    item->SetMetadata(key_line.substr(9), value_line.substr(11));
                }
            }

            std::string end_marker;
            std::getline(file, end_marker);
            AddItem(item);
        }

        return true;
    }

    bool Loaf::Save (const std::string& filepath) const {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        file << name_ << "\n";
        file << description_ << "\n";
        file << layout_ << "\n";

        file << runtime_rules_.size() << "\n";
        for (const auto& rule : runtime_rules_) {
            file << rule.first << "\n";
            file << rule.second << "\n";
        }

        file << items_.size() << "\n";
        for (const auto& item : items_) {
            if (!item) {
                continue;
            }

            file << "TYPE:" << ToTypeString(item->GetType()) << "\n";
            file << "ID:" << item->GetId() << "\n";
            file << "NAME:" << item->GetName() << "\n";
            file << "PATH:" << item->GetPath() << "\n";

            const std::map<std::string, std::string> metadata = {
                {"depends_on", item->GetMetadata("depends_on")},
                {"args", item->GetMetadata("args")},
                {"working_dir", item->GetMetadata("working_dir")},
                {"auto_start", item->GetMetadata("auto_start")}
            };

            size_t metadata_count = 0;
            for (const auto& pair : metadata) {
                if (!pair.second.empty()) {
                    ++metadata_count;
                }
            }

            file << "META_COUNT:" << metadata_count << "\n";
            for (const auto& pair : metadata) {
                if (pair.second.empty()) {
                    continue;
                }
                file << "META_KEY:" << pair.first << "\n";
                file << "META_VALUE:" << pair.second << "\n";
            }

            file << "END_ITEM\n";
        }

        return true;
    }

    void Loaf::Clear ( ) {
        items_.clear();
        runtime_rules_.clear();
        running_ = false;
    }

    void Loaf::AddItem (std::shared_ptr<LoafItem> item) {
        if (item) {
            items_.push_back(item);
        }
    }

    void Loaf::RemoveItem (const std::string& itemId) {
        items_.erase(
            std::remove_if(items_.begin(), items_.end(),
                [&itemId](const std::shared_ptr<LoafItem>& item) {
                    return item && item->GetId() == itemId;
                }),
            items_.end()
        );
    }

    std::shared_ptr<LoafItem> Loaf::GetItem (const std::string& itemId) const {
        for (const auto& item : items_) {
            if (item && item->GetId() == itemId) {
                return item;
            }
        }
        return nullptr;
    }

    const std::vector<std::shared_ptr<LoafItem>>& Loaf::GetItems ( ) const {
        return items_;
    }

    void Loaf::SetName (const std::string& name) {
        name_ = name;
    }

    std::string Loaf::GetName ( ) const {
        return name_;
    }

    void Loaf::SetDescription (const std::string& description) {
        description_ = description;
    }

    std::string Loaf::GetDescription ( ) const {
        return description_;
    }

    void Loaf::SetLayout (const std::string& layout) {
        layout_ = layout;
    }

    std::string Loaf::GetLayout ( ) const {
        return layout_;
    }

    void Loaf::SetRuntimeRule (const std::string& key, const std::string& value) {
        runtime_rules_[key] = value;
    }

    std::string Loaf::GetRuntimeRule (const std::string& key) const {
        auto it = runtime_rules_.find(key);
        return (it != runtime_rules_.end()) ? it->second : "";
    }

    bool Loaf::Run ( ) {
        if (running_) {
            return false;
        }

        for (const auto& item : items_) {
            if (item && !item->Execute()) {
                return false;
            }
        }

        running_ = true;
        return true;
    }

    bool Loaf::Stop ( ) {
        if (!running_) {
            return false;
        }

        running_ = false;
        return true;
    }

    bool Loaf::IsRunning ( ) const {
        return running_;
    }

} // namespace BreadBin