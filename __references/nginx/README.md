# nginx reference server

A throwaway nginx instance for diffing behaviour against `webserv`.
Runs **inside the webserv dev container** (same place you build and run the project).

- Package: `nginx-light` (Ubuntu 22.04), nginx 1.18.0 — added in
  [`.devcontainer/Dockerfile`](../../.devcontainer/Dockerfile)
- Config: [`nginx.conf`](nginx.conf) — self-contained, all paths relative to the
  project root; `mime.types` is vendored next to it
- Listens on **http://localhost:8084** (a forwarded port; webserv uses 8080)
- Document root: the project's `./html` directory (same files webserv serves)

## Usage

From inside the container, at the project root:

```sh
./__references/nginx/nginx.sh start     # background
./__references/nginx/nginx.sh run       # foreground, Ctrl-C to quit
./__references/nginx/nginx.sh reload    # after editing nginx.conf
./__references/nginx/nginx.sh test      # syntax check
./__references/nginx/nginx.sh stop
```

Or drive it from the macOS host:

```sh
docker exec -w /workspaces/42-webserv 42-webserv ./__references/nginx/nginx.sh start
```

Logs: `__references/nginx/logs/{access,error}.log` (gitignored).

## If you rebuild the container

`nginx-light` is in the Dockerfile, so a rebuild installs it automatically.
It was also installed live in the current container, so it already works without
a rebuild.

## What the config mirrors

| webserv config                 | nginx equivalent here                   |
|--------------------------------|-----------------------------------------|
| `configs/autoindex.conf`       | `autoindex on;` on `location /`          |
| `configs/error_page_test.conf` | server- and location-level `error_page`  |
| `configs/redirections.conf`    | `/example` 301, `/test` `/html` 302      |
| `client_max_body_size 10m`     | same                                    |

## Comparing responses

```sh
diff <(curl -si localhost:8080/nope) <(curl -si localhost:8084/nope)
```

Expect differences in the `Server:` header, `Date`, and connection handling —
focus on the status line, body, and content headers.
