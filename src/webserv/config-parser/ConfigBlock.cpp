#include "webserv/config-parser/ConfigBlock.hpp"

ConfigBlock::ConfigBlock(std::string name,
                         BlockType type,
                         std::string value,
                         ConfigBlock* parent)
  : _parent(parent)
  , _name(name)
  , _value(value)
  , _type(type)
{}

const ConfigBlock::DirectiveMap&
ConfigBlock::getDirectiveMap(void) const
{
    return _directives;
}

ConfigBlock*
ConfigBlock::getParent(void) const
{
    return _parent;
}

const std::string&
ConfigBlock::getName(void) const
{
    return _name;
}

const std::string&
ConfigBlock::getValue(void) const
{
    return _value;
}

ConfigBlock::BlockType
ConfigBlock::getType(void) const
{
    return _type;
}

ConfigBlock::~ConfigBlock(void)
{
    for (std::vector<ConfigBlock*>::const_iterator ite = _blocks.begin();
         ite != _blocks.end();
         ++ite) {
        delete *ite;
    }
}