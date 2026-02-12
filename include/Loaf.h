#ifndef LOAF_H
#define LOAF_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include "LoafItem.h"

namespace BreadBin {

    class Loaf {
        public:
            Loaf();
            explicit Loaf(const std::string& name);
            ~Loaf();

            bool load(const std::string& filepath);
            bool save(const std::string& filepath) const;
            void clear();

            void addItem(std::shared_ptr<LoafItem> item);
            void removeItem(const std::string& itemId);
            std::shared_ptr<LoafItem> getItem(const std::string& itemId) const;
            const std::vector<std::shared_ptr<LoafItem>>& getItems() const;

            void setName(const std::string& name);
            std::string getName() const;
            void setDescription(const std::string& description);
            std::string getDescription() const;

            void setLayout(const std::string& layout);
            std::string getLayout() const;
            void setRuntimeRule(const std::string& key, const std::string& value);
            std::string getRuntimeRule(const std::string& key) const;

            bool run();
            bool stop();
            bool isRunning() const;

        private:
            std::string name_;
            std::string description_;
            std::string layout_;
            std::vector<std::shared_ptr<LoafItem>> items_;
            std::map<std::string, std::string> runtimeRules_;
            bool running_;
    };

} // namespace BreadBin

#endif // LOAF_H
