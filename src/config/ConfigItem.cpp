#include "config/ConfigItem.hpp"
#include <iomanip>
#include <stdexcept>

ConfigItem::ConfigItem(const std::string& name,
                       BlockType blockType,
                       ConfigItem* parent,
                       const std::string& value)
  : _name(name)
  , _value(value)
  , _parent(parent)
  , _blockType(blockType)
{}

ConfigItem::~ConfigItem(void)
{
    for (std::vector<ConfigItem*>::const_iterator ite = children.begin();
         ite != children.end();
         ++ite) {
        delete *ite;
    }
}

BlockType
ConfigItem::getType(void) const
{
    return _blockType;
}

const std::string&
ConfigItem::getName(void) const
{
    return _name;
}

const std::string&
ConfigItem::getValue(void) const
{
    return _value;
}

ConfigItem*
ConfigItem::getParent(void) const
{
    return _parent;
}

const ConfigItem*
ConfigItem::findNearest(const std::string& key) const
{
    const ConfigItem *current = getType() == NOT_A_BLOCK ? getParent() : this,
                     *atom = 0;

    while (current) {
        atom = current->findAtomInBlock(key);
        if (atom) {
            return atom;
        }
        current = current->getParent();
    }
    return 0;
}

ConfigItem*
ConfigItem::findNearest(const std::string& key)
{
    return const_cast<ConfigItem*>(
      static_cast<const ConfigItem*>(this)->findNearest(key));
}

const ConfigItem*
ConfigItem::findAtomInBlock(const std::string& key) const
{
    if (getType() == NOT_A_BLOCK) {
        throw std::runtime_error(
          "findAtomInBlock can only be used on config blocks");
    }

    for (std::vector<ConfigItem*>::const_iterator ite = children.begin();
         ite != children.end();
         ++ite) {
        if ((*ite)->getName() == key) {
            return *ite;
        }
    }

    return 0;
}

ConfigItem*
ConfigItem::findAtomInBlock(const std::string& key)
{
    return const_cast<ConfigItem*>(
      static_cast<const ConfigItem*>(this)->findAtomInBlock(key));
}

const std::vector<const ConfigItem*>
ConfigItem::findBlocks(const std::string& key) const
{
    std::vector<const ConfigItem*> blocks;

    if (getType() == NOT_A_BLOCK) {
        throw std::runtime_error(
          "findBlocks can only be used on config blocks");
    }

    for (std::vector<ConfigItem*>::const_iterator ite = children.begin();
         ite != children.end();
         ++ite) {
        if ((*ite)->getName() == key) {
            blocks.push_back(*ite);
        }
    }

    return blocks;
}

const std::vector<ConfigItem*>
ConfigItem::findBlocks(const std::string& key)
{
    std::vector<ConfigItem*> blocks;

    if (getType() == NOT_A_BLOCK) {
        throw std::runtime_error(
          "findBlocks can only be used on config blocks");
    }

    for (std::vector<ConfigItem*>::const_iterator ite = children.begin();
         ite != children.end();
         ++ite) {
        if ((*ite)->getName() == key) {
            blocks.push_back(*ite);
        }
    }

    return blocks;
}

std::ostream&
operator<<(std::ostream& os, const ConfigItem& item)
{
    os << "{\n";

    for (std::vector<ConfigItem*>::const_iterator ite = item.children.begin();
         ite != item.children.end();
         ++ite) {
        os << "\t" << std::left << std::setw(15) << (*ite)->getName() << " "
           << (*ite)->getValue()
           << ((*ite)->getType() != NOT_A_BLOCK ? "(BLOCK)" : "") << "\n";
    }

    return os << "}\n";
}
