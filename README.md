![build](https://github.com/busshi/webserv/actions/workflows/compil.yml/badge.svg)

# Webserv

A tiny HTTP server implementation built with C++, for learning purpose.

## Documentation
- Configuration file parsing
  - File format
    - Example configuration
    - File format rules
      - Lexer rules


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