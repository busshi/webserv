#pragma once
#include <map>
#include <string>
#include <vector>

class ConfigBlock
{
  public:
    typedef std::map<std::string, std::string> DirectiveMap;

    enum BlockType
    {
        BLOCK_GLOBAL = 1 << 0,
        BLOCK_LOCATION = 1 << 1,
        BLOCK_SERVER = 1 << 2
    };

  private:
    DirectiveMap _directives;
    ConfigBlock* _parent;
    std::string _name, _value;
    BlockType _type;

  public:
    std::vector<ConfigBlock*> _blocks;
    ConfigBlock(std::string name,
                BlockType type,
                std::string value = "",
                ConfigBlock* parent = 0);
    ~ConfigBlock(void);

    void addDirective(const DirectiveMap::value_type& value);

    const DirectiveMap& getDirectiveMap(void) const;
    ConfigBlock* getParent(void) const;
    const std::string& getName(void) const;
    const std::string& getValue(void) const;
    BlockType getType(void) const;
};
