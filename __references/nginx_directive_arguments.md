# Nginx Directive Arguments & Syntax Reference Guide

This document specifies the exact argument counts, allowed argument types, syntax formats, and validation rules for Nginx directives.

---

## 1. Summary Table of Directive Arguments

| Directive | Type | Argument Count | Syntax Structure | Argument Description / Examples |
| :--- | :--- | :--- | :--- | :--- |
| **`server`** | Block | **0** | `server { ... }` | Accepts no arguments; contains directives inside `{}`. |
| **`location`** | Block | **1 or 2** | `location [modifier] uri { ... }` | 1 arg: `uri` (prefix matching)<br>2 args: `modifier` (`=`, `^~`, `~`, `~*`) + `uri` |
| **`listen`** | Simple | **1 to 2** | `listen address[:port] [default_server];` | Arg 1: Port (`8080`), IP (`127.0.0.1`), or `IP:Port`<br>Arg 2 (optional): `default_server` flag |
| **`server_name`** | Simple | **1 or more** | `server_name name1 [name2 ...];` | Space-delimited list of hostnames/domains (e.g., `example.com *.example.com`) |
| **`root`** | Simple | **1** | `root path;` | Directory path string (e.g., `/var/www/html`) |
| **`alias`** | Simple | **1** | `alias path;` | Directory replacement path string (e.g., `/var/www/images/`) |
| **`index`** | Simple | **1 or more** | `index file1 [file2 ...];` | Space-delimited list of fallback index filenames (e.g., `index.html index.htm`) |
| **`client_max_body_size`**| Simple | **1** | `client_max_body_size size;` | Size string with optional unit suffix (`k`/`K`, `m`/`M`, `g`/`G`, or `0` for unlimited) |
| **`error_page`** | Simple | **2 or more** | `error_page code [...] [=response] uri;` | At least 1 HTTP status code + 1 target URI (e.g., `404 /404.html` or `500 502 /5xx.html`) |
| **`return`** | Simple | **1 or 2** | `return code [text\|URL];` or `return URL;` | 1 arg: Status code or redirect URL<br>2 args: Status code + response body text or redirect URL |
| **`if`** | Block | **1** | `if (condition) { ... }` | Single condition block enclosed in parentheses |

---

## 2. Detailed Directive Specifications

### 2.1. Block Directives

#### `server`
* **Argument Count**: Exactly **0**
* **Syntax**: `server { ... }`
* **Parsing Validation**:
  * Throw an error if any tokens appear between `server` and the opening brace `{`.

#### `location`
* **Argument Count**: **1 or 2**
* **Syntax**:
  * 1 argument: `location uri { ... }`
  * 2 arguments: `location modifier uri { ... }`
* **Allowed Modifiers**:
  * `=` : Exact match
  * `^~` : Preferential prefix match
  * `~` : Case-sensitive regex match
  * `~*` : Case-insensitive regex match
* **Parsing Validation**:
  * If 2 arguments are provided, check if Argument 1 is a valid modifier (`=`, `^~`, `~`, `~*`).
  * If Argument 1 is not a recognized modifier, treat it as a syntax error or part of an invalid URI string.

#### `if`
* **Argument Count**: **1 condition expression**
* **Syntax**: `if (condition) { ... }`
* **Parsing Validation**:
  * Condition expression must be enclosed in parentheses `(...)`.
  * Typical conditions evaluate variables against values (e.g., `$request_method = POST`).

---

### 2.2. Simple Directives

#### `listen`
* **Argument Count**: **1 or 2**
* **Syntax**: `listen address[:port] [default_server];`
* **Examples**:
  * `listen 80;` (1 arg: Port only)
  * `listen 127.0.0.1:8080;` (1 arg: IP and Port)
  * `listen 8080 default_server;` (2 args: Port + flag)
* **Parsing Validation**:
  * Argument 1 must be parsed into IP address and/or Port number.
  * If Argument 2 exists, verify it equals recognized keywords like `default_server` or `ssl`.

#### `server_name`
* **Argument Count**: **1 or more** (variadic)
* **Syntax**: `server_name name1 [name2 ...];`
* **Examples**:
  * `server_name localhost;` (1 arg)
  * `server_name example.com www.example.com;` (2 args)
* **Parsing Validation**:
  * Must have at least 1 hostname argument before the trailing semicolon `;`.

#### `root`
* **Argument Count**: Exactly **1**
* **Syntax**: `root path;`
* **Examples**:
  * `root /var/www/html;`
* **Parsing Validation**:
  * Must have exactly 1 path parameter.
  * Fail parsing if 0 or >1 parameters are supplied.

#### `alias`
* **Argument Count**: Exactly **1**
* **Syntax**: `alias path;`
* **Examples**:
  * `alias /data/images/;`
* **Parsing Validation**:
  * Must have exactly 1 path parameter.
  * Must only exist inside a `location` context block.

#### `index`
* **Argument Count**: **1 or more** (variadic)
* **Syntax**: `index file1 [file2 ...];`
* **Examples**:
  * `index index.html;` (1 arg)
  * `index index.html index.htm index.php;` (3 args)
* **Parsing Validation**:
  * Must provide at least 1 filename argument.

#### `client_max_body_size`
* **Argument Count**: Exactly **1**
* **Syntax**: `client_max_body_size size;`
* **Examples**:
  * `client_max_body_size 10m;`
  * `client_max_body_size 1024k;`
  * `client_max_body_size 0;` (disables body size checking)
* **Parsing Validation**:
  * Argument must be a numeric value optionally followed by a unit suffix (`k`/`K` for kilobytes, `m`/`M` for megabytes, `g`/`G` for gigabytes).

#### `error_page`
* **Argument Count**: **2 or more**
* **Syntax**: `error_page code [...] [=response] uri;`
* **Examples**:
  * `error_page 404 /404.html;` (2 args: 1 status code + 1 URI)
  * `error_page 500 502 503 504 /5xx.html;` (5 args: 4 status codes + 1 URI)
  * `error_page 404 =200 /empty.gif;` (3 args: code + status override + URI)
* **Parsing Validation**:
  * All leading arguments except the last one must be valid HTTP status codes (integers 300-599) or status override syntax (`=response`).
  * The final argument must be the URI path string.

#### `return`
* **Argument Count**: **1 or 2**
* **Syntax**:
  * `return code;`
  * `return code text;`
  * `return code URL;`
  * `return URL;`
* **Examples**:
  * `return 404;` (1 arg: status code)
  * `return 301 https://example.com$request_uri;` (2 args: code + URL)
  * `return 200 "OK";` (2 args: code + body text)
  * `return https://example.com;` (1 arg: implicit 302 redirect URL)
* **Parsing Validation**:
  * Validate if argument is an integer status code or a URL string starting with `http://` or `https://`.

---

## 3. Quick Reference Matrix for Argument Counts

| Directive | Min Arguments | Max Arguments | Argument Types |
| :--- | :--- | :--- | :--- |
| `server` | 0 | 0 | None |
| `location` | 1 | 2 | String (Modifier), String (URI) |
| `listen` | 1 | 2 | String (IP/Port), Keyword (`default_server`) |
| `server_name` | 1 | $\infty$ | String (Hostnames) |
| `root` | 1 | 1 | String (Path) |
| `alias` | 1 | 1 | String (Path) |
| `index` | 1 | $\infty$ | String (Filenames) |
| `client_max_body_size` | 1 | 1 | String / Integer + Unit (`10m`, `128k`) |
| `error_page` | 2 | $\infty$ | Integer(s) (Status Codes) + String (URI) |
| `return` | 1 | 2 | Integer (Status Code) / String (URL or Text) |
| `if` | 1 | 1 | Condition expression |
