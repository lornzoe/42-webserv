# Nginx Directive Context & Restriction Reference Guide

This document details the directive restrictions, valid contexts, allowed frequencies (cardinality), syntax rules, and inheritance behavior for Nginx directives as used in standard Nginx configurations and HTTP web server implementations (e.g., 42 Webserv).

---

## 1. Overview of Configuration Contexts

Nginx directives operate within specific hierarchical contexts (scopes). Understanding these contexts is essential for validating configuration files.

```
+-------------------------------------------------------------------+
| main / global context                                             |
|                                                                   |
|   http {                                                          |
|       # http context                                              |
|                                                                   |
|       server {                                                    |
|           # server context                                        |
|                                                                   |
|           location / {                                            |
|               # location context                                  |
|               if (...) {                                          |
|                   # if context                                    |
|               }                                                   |
|           }                                                       |
|       }                                                           |
|   }                                                               |
+-------------------------------------------------------------------+
```

### Context Definitions
* **`main` / Global**: The root of the configuration file. Defines global process settings.
* **`http`**: Encloses directives for configuring HTTP network traffic handling.
* **`server`**: Defines a virtual server handling requests for specific hostnames or IP/ports.
* **`location`**: Configures request URI matching and routing rules inside a `server` block.
* **`if`**: Conditional execution block evaluated during request processing (typically inside `server` or `location`).

---

## 2. Summary Table of Directive Restrictions

| Directive | Type | Allowed Contexts | Max Count per Context | Inherited? |
| :--- | :--- | :--- | :--- | :--- |
| **`server`** | Block | `http` | Multiple | N/A |
| **`location`** | Block | `server`, `location` | Multiple | N/A |
| **`listen`** | Simple | `server` | Multiple (Unique IP:Port) | No |
| **`server_name`** | Simple | `server` | Multiple (or space-delimited) | No |
| **`root`** | Simple | `http`, `server`, `location`, `if` | 1 per context | Yes (override) |
| **`alias`** | Simple | `location` | 1 per location | No (replaces `root`) |
| **`index`** | Simple | `http`, `server`, `location` | 1 per context (multiple files) | Yes (override) |
| **`client_max_body_size`** | Simple | `http`, `server`, `location` | 1 per context | Yes (override) |
| **`error_page`** | Simple | `http`, `server`, `location`, `if` | Multiple (unique status codes) | Yes (override) |
| **`return`** | Simple | `server`, `location`, `if` | 1 per execution branch | No |
| **`if`** | Block | `server`, `location` | Multiple | N/A |

---

## 3. Detailed Directive Specifications

### 3.1. Block Directives

#### `server`
* **Syntax**: `server { ... }`
* **Type**: Block directive
* **Valid Contexts**: `http`
* **Cardinality**: 1 or more per `http` block.
* **Restrictions & Behavior**:
  * Must contain at least one `listen` directive to bind to a port/address.
  * Cannnot be nested inside another `server` block or a `location` block.

#### `location`
* **Syntax**: `location [ = | ~ | ~* | ^~ ] uri { ... }`
* **Type**: Block directive
* **Valid Contexts**: `server`, `location` (nested locations)
* **Cardinality**: Multiple per `server` context.
* **Restrictions & Behavior**:
  * Cannot exist in `http` or `main` context without a surrounding `server` block.
  * Modifiers define matching precedence:
    * `=` : Exact match.
    * `^~` : Preferential prefix match.
    * `~` : Case-sensitive regex match.
    * `~*` : Case-insensitive regex match.

#### `if`
* **Syntax**: `if (condition) { ... }`
* **Type**: Block directive
* **Valid Contexts**: `server`, `location`
* **Cardinality**: Multiple per context.
* **Restrictions & Behavior**:
  * Care must be taken with `if` inside `location` due to Nginx request processing phase behavior ("If is Evil" in Nginx context).
  * Directives inside `if` override outer scope settings when the condition evaluates to true.

---

### 3.2. Simple Directives

#### `listen`
* **Syntax**: `listen address[:port];` or `listen port;`
* **Type**: Simple directive (terminated with `;`)
* **Valid Contexts**: `server`
* **Cardinality**: 1 or more per `server` block.
* **Restrictions & Behavior**:
  * Standard default port is `80` if unspecified.
  * Multiple `server` blocks can share the same port as long as `server_name` directives differentiate them (virtual host routing).

#### `server_name`
* **Syntax**: `server_name name1 [name2 ...];`
* **Type**: Simple directive
* **Valid Contexts**: `server`
* **Cardinality**: 0 or 1 per `server` block (or multiple space-delimited values).
* **Restrictions & Behavior**:
  * Used for HTTP `Host` header matching to route requests to the correct `server` block.
  * Supports exact names, wildcards (`*.example.com`), and regular expressions.

#### `root`
* **Syntax**: `root path;`
* **Type**: Simple directive
* **Valid Contexts**: `http`, `server`, `location`, `if`
* **Cardinality**: Maximum 1 per context scope.
* **Restrictions & Behavior**:
  * Constructs full file path by appending Request URI to the specified `root` path:
    `Full Path = root + Request-URI`
  * Mutually exclusive with `alias` within the same `location` block.

#### `alias`
* **Syntax**: `alias path;`
* **Type**: Simple directive
* **Valid Contexts**: `location` **ONLY**
* **Cardinality**: Maximum 1 per `location` block.
* **Restrictions & Behavior**:
  * Replaces the matching URI prefix with the specified path:
    `Full Path = alias + (Request-URI minus Location-Prefix)`
  * **Restriction**: Cannot be used in `server` or `http` contexts.
  * **Restriction**: Cannot coexist with `root` in the same `location` block.

#### `index`
* **Syntax**: `index file1 [file2 ...];`
* **Type**: Simple directive
* **Valid Contexts**: `http`, `server`, `location`
* **Cardinality**: 1 directive per block (can list multiple fallback files).
* **Restrictions & Behavior**:
  * Server checks files in order of listing when a directory is requested.

#### `client_max_body_size`
* **Syntax**: `client_max_body_size size;` (e.g., `10m`, `128k`, `0` for unlimited)
* **Type**: Simple directive
* **Valid Contexts**: `http`, `server`, `location`
* **Cardinality**: 1 per context block.
* **Restrictions & Behavior**:
  * Sets the maximum allowed request body size (`Content-Length`).
  * Returns `413 Payload Too Large` (or `413 Content Too Large` in HTTP/1.1 RFC 9110) if client request exceeds this limit.

#### `error_page`
* **Syntax**: `error_page code [...] [=response] uri;`
* **Type**: Simple directive
* **Valid Contexts**: `http`, `server`, `location`, `if`
* **Cardinality**: Multiple directives allowed.
* **Restrictions & Behavior**:
  * Defines custom URI or static file to serve when specific HTTP error status codes occur (e.g., `error_page 404 /404.html;`).
  * Can map multiple error codes at once: `error_page 500 502 503 504 /5xx.html;`.

#### `return`
* **Syntax**: `return code [text];` or `return code URL;` or `return URL;`
* **Type**: Simple directive
* **Valid Contexts**: `server`, `location`, `if`
* **Cardinality**: 1 per execution block path.
* **Restrictions & Behavior**:
  * Stops further request processing immediately and returns specified status code / redirect response to the client.

---

## 4. Directive Inheritance Rules

Nginx uses a **top-down inheritance model**:

1. **Outer-to-Inner Propagation**: Directives set in higher contexts (`http` -> `server` -> `location`) are inherited by child contexts unless explicitly redefined.
2. **Override Mechanism**: Defining a directive in a child context completely overrides any inherited setting from the parent context for that child scope.
3. **Array Directives**: Directives like `error_page` will override parent definitions completely if specified in a child context rather than appending to them.

---

## 5. Quick Reference Matrix for Web Server Validation

| Scope / Context | Directives Allowed |
| :--- | :--- |
| **`main`** | `http` |
| **`http`** | `server`, `root`, `index`, `client_max_body_size`, `error_page` |
| **`server`** | `listen`, `server_name`, `location`, `root`, `index`, `client_max_body_size`, `error_page`, `return`, `if` |
| **`location`** | `location` (nested), `alias`, `root`, `index`, `client_max_body_size`, `error_page`, `return`, `if` |
| **`if`** | `root`, `error_page`, `return` |
