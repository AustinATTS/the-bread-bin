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
            Loaf ( );
            explicit Loaf (const std::string& name);
            ~Loaf ( );

            bool Load (const std::string& filepath);
            [[nodiscard]] bool Save (const std::string& filepath) const;
            void Clear ( );
            void AddItem (std::shared_ptr<LoafItem> item);
            void RemoveItem (const std::string& itemId);
            [[nodiscard]] std::shared_ptr<LoafItem> GetItem (const std::string& itemId) const;
            [[nodiscard]] const std::vector<std::shared_ptr<LoafItem>>& GetItems ( ) const;
            void SetName (const std::string& name);
            [[nodiscard]] std::string GetName ( ) const;
            void SetDescription (const std::string& description);
            [[nodiscard]] std::string GetDescription ( ) const;
            void SetLayout (const std::string& layout);
            [[nodiscard]] std::string GetLayout ( ) const;
            void SetRuntimeRule (const std::string& key, const std::string& value);
            [[nodiscard]] std::string GetRuntimeRule (const std::string& key) const;
            bool Run ( );
            bool Stop ( );
            [[nodiscard]] bool IsRunning ( ) const;

        private:
            std::string name_;
            std::string description_;
            std::string layout_;
            std::vector<std::shared_ptr<LoafItem>> items_;
            std::map<std::string, std::string> runtime_rules_;
            bool running_;
    };
} // namespace BreadBin

#endif // LOAF_H