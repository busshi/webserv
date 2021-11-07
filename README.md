![build](https://github.com/busshi/webserv/actions/workflows/compil.yml/badge.svg)

[![aldubar's 42Project Score](https://badge42.herokuapp.com/api/project/aldubar/webserv)](https://github.com/JaeSeoKim/badge42)

# Webserv

A tiny HTTP server implementation built with C++, for learning purpose.

## Documentation
- [Configuration file parsing](#configuration-file-parsing)
  - [File format](#file-format)
    - [Example configuration](#example-configuration)
    - [Lexer rules](#lexer-rules)
    - [Configuration file is free form](#configuration-file-is-free-form)
    - [Comments](#Comments)
  - [How parsing is done, and data actually used](#how-parsing-is-done-and-data-actually-used)
    - [Data representation](#data-representation)
    - [Using the data](#using-the-data)
    - [`findBlocks`](#findBlocks)
    - [`findAtomInBlock`](#findatominblock)
    - [`findNearestAtom`](#findnearestatom)

# Configuration file parsing

Such as any HTTP server, our `webserv` needs a configuration file to operate.

## File format

### Example configuration

For that purpose, we decided to replicate the `nginx` configuration style, which is basically made of key-value pairs and blocks.

Here is a minimal example configuration:

```nginx
server {
    listen 80;
    index index.html;

    location / {
        root /var/www/html;
    }

    location /blog {
        root /var/www/html/blog;
    }
}
```

This should give approximatively the same result whether `nginx` or our `webserv` is used.

### File format rules

>As there is no official specification for the file format `nginx` uses, our version may slightly differ, which is not an issue.

#### Lexer rules

The lexer is responsible for validating the grammatical structure of the configuration file.

Here are some of the most common rules enforced by our lexer:

- A configuration option *which is not a block* must always have a key and a value, and then end with a semicolon character `;`.

```nginx
autoindex;     # error: no value
autoindex on   # error: key-value pair but no ending semicolon
autoindex off; # valid!
```

- A configuration option *which **IS** a block* *must* have a key, but **not necessarily** a value. If a value is provided, its end is delimited by the first `{` encountered. Block scope must be represented by a pair of curly braces `{}`, inside which can be put more specific configuration options. Putting a *semicolon after the closing brace* is not allowed and would produce a lexing error. If either the opening or closing brace is forgotten, the related error message may not make much sense as it will cascade all over the remaining of the configuration file.

```nginx
# error: anonymous blocks make no sense
{
}

# error: semicolon at the end
server {
    autoindex on;
};

# ok
server {}

# ok
server { autoindex on; }

#ok
server
{
}

# ok: 2 empty server blocks, and one that contains two location blocks
server {} server {} server {
    location / {}
    location /blog {
        root /var/www/html/blog;
    }
}
```

#### Configuration file is free form

As demonstrated above, there is almost no requirement on which form or style the configuration file must use. Lexically, most tokens can be separated with any amount of spaces and newlines without causing any issue. The only real exception is that non-block configuration items must put the semicolon that ends the value on the same line than the value.

For example:
```nginx
server     {
      autoindex

           on       ;
 }
```

is perfectly valid, while:

```nginx
server {
    autoindex
    on
    ;
}
```

is not, because the semicolon is not on the same line than the value.

#### Comments

When the `#` character is encountered, the remaining characters on the line are skipped.

### How parsing is done, and data actually used

#### Data representation

Each configuration item (block or non-block) shares the same underlying structure, and is represented as a simple C++ class:

```cpp
struct ConfigItem {
    std::string name;
    std::string value;
    std::vector<ConfigItem*> children;
    BlockType type;
}
```
> This is a highly simplified version, please see the corresponding header for more informations.

As shown by the above structure, the block system is put in place by creating a vector of `ConfigItem` in each `ConfigItem`. If a `ConfigItem` has at least one child, then it means that it is a block, because only blocks are capable of holding other items. The `type` is used to know which block - if it is a block - it is.

#### Using the data

In reality, the `ConfigItem` is a way more complex class that bundles several methods in order to easily access its child or parent elements.

First, the configuration is parsed from a file, using a `ConfigParser` object:

```cpp
ConfigParser cfgp;

ConfigItem* global = cfgp.loadConfig("/path/to/config");
```

During the load process, a `Lexer::LexerException` or a `ConfigParser::ParserException` may be thrown if there is something wrong with the lexing or parsing processes.

The returned `ConfigItem*` is a pointer to the global scope of the configuration, which therefore holds the whole configuration.

##### findBlocks

Helper method that returns a `ConfigItem*` vector holding all the blocks of a given type that are direct children of the current block. The `ConfigItem` on which this method is called must therefore be a block otherwise an exception is thrown.

In the following example, `findBlocks` is used to process each `server` block from the global config scope, then each `location` block of each `server` block.

```cpp
std::vector<ConfigItem*> serverBlocks = global->findBlocks("server");

for (std::vector<ConfigItem*>::const_iterator ite = serverBlocks.begin();
    ite != serverBlocks.end(); ++ite) {
        std::vector<ConfigItem*> locationBlocks = (*ite)->findBlocks("location");
        // do something with each location of this server block
}
```

##### findAtomInBlock

Retrieve a given `ConfigItem*` inside the block item on which this method is called.

```cpp
ConfigItem* globalAutoindex = global->findAtomInBlock("autoindex");

if (globalAutoindex) {
    std::cout << globalAutoindex->name() << " = " << globalAutoindex->value();
} else {
    std::cout << "Did not found autoindex directly inside global scope\n";
}
```

##### findNearestAtom

Retrieve the *nearest* configuration item which may apply to the one on which this method is called.

Given the following configuration:

```nginx
some_config 2;

server {
    some_config 1;
    location / {
        some_config 0;
    }
}
```

Assuming that `locationBlock` refers to the location configuration item as described by the configuration above:

```cpp
ConfigItem* someConfig = locationBlock->findNearestAtom("some_config");

std::cout << someConfig->value() << "\n";
```

This code snippet will print `1` because it is the nearest value, starting at the scope of the location item itself.

In case we edit the configuration file like so:

```nginx
some_config 2;

server {
    location / {
        some_config 0;
    }
}
```

Then the printed value will be `2` as this is the nearest value now that `some_config` is no longer defined in the `server` block. This may become really handy for some configuration options that are able to defined at multiple scope levels.
