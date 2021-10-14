#pragma once
#include <iostream>
#include <ostream>
#include <stdint.h>
#include <string>
#include <vector>

enum BlockType
{
    NOT_A_BLOCK = 0,
    BLOCK_SERVER = 1 << 0,
    BLOCK_LOCATION = 1 << 1,
    BLOCK_GLOBAL = 1 << 2
};

class ConfigItem
{
    std::string _name, _value;
    ConfigItem* _parent;
    BlockType _blockType;

    /* deleted for now */
    ConfigItem(const ConfigItem& other);
    ConfigItem& operator=(const ConfigItem& rhs);

  public:
    ConfigItem(const std::string& name,
               BlockType type,
               ConfigItem* parent,
               const std::string& value = "");
    ~ConfigItem(void);

    std::vector<ConfigItem*> children;

    BlockType getType(void) const;
    ConfigItem* getParent(void) const;
    const std::string& getName(void) const;
    const std::string& getValue(void) const;

    ConfigItem* findNearestAtom(const std::string& key) const;
    ConfigItem* findAtom(const std::string& key);
    std::vector<ConfigItem*> findBlocks(const std::string& key);
};

std::ostream&
operator<<(std::ostream& os, const ConfigItem& item);

struct FindConfigItemPredicate
{
    std::string key;
    FindConfigItemPredicate(const std::string& name)
      : key(name)
    {}
    bool operator()(const ConfigItem* item) const
    {
        return item->getName() == key;
    }
};