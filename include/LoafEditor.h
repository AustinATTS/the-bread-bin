#ifndef LOAF_EDITOR_H
#define LOAF_EDITOR_H

#include <string>
#include <memory>
#include "Loaf.h"

namespace BreadBin {

    class LoafEditor {
        public:
            LoafEditor();
            ~LoafEditor();

            void newLoaf(const std::string& name);
            bool openLoaf(const std::string& filepath);
            bool saveLoaf(const std::string& filepath);
            bool closeLoaf();

            bool addApplication(const std::string& id, const std::string& name, const std::string& path);
            bool addFile(const std::string& id, const std::string& name, const std::string& path);
            bool addConfig(const std::string& id, const std::string& name, const std::string& path);
            bool addScript(const std::string& id, const std::string& name, const std::string& path);
            bool addWebPage(const std::string& id, const std::string& name, const std::string& url);
            bool removeItem(const std::string& itemId);

            bool setLoafName(const std::string& name);
            bool setLoafDescription(const std::string& description);
            bool setLoafLayout(const std::string& layout);
            bool setRuntimeRule(const std::string& key, const std::string& value);

            std::shared_ptr<Loaf> getCurrentLoaf() const;
            bool hasUnsavedChanges() const;

        private:
            std::shared_ptr<Loaf> currentLoaf_;
            bool unsavedChanges_;
    };

} // namespace BreadBin

#endif // LOAF_EDITOR_H
