# webserv showcase
## Usage
```bash
make && ./webserv showcase/full.conf    # from the repo root
H=localhost:8080    # set env
```

## Site A — `0.0.0.0:8080`, `127.0.0.1:8081`

Root `showcase/www`, body limit 10m, custom error pages, CGI for `.py` and `.pl`.

| Location | Methods | Purpose | Verify |
|---|---|---|---|
| `/` | GET POST DELETE | Landing page listing every feature; also serves the folders below that have no location of their own | Browser: open `http://localhost:8080/` |
| `/static/` | (via `/`) | Sample files for MIME types (jpg, txt, json, csv, svg, webp) | `curl -s -D- -o /dev/null $H/static/sample.json \| grep -i content-type` → `application/json` |
| `/playground/` | (via `/`) | Request tester: send any method/path/body, see status + headers | Browser: `/playground/`, click a preset button |
| `/uploads/` | (via `/`) | Upload page: multi-file upload, list and delete (targets `/storage/`) | Browser: `/uploads/`, upload two files, tick them, Delete selected |
| `/cgi/` | (via `/`) | Python + Perl list apps and the CGI demo scripts | Browser: `/cgi/`, add and remove an item in each list |
| `/errors/` | (via `/`) | The custom error pages | `curl $H/errors/404.html` (served as a normal page, 200) |
| `/files/` | GET | Directory listing (`autoindex on`) | Browser: `/files/` shows "Index of /files/" |
| `/noindex/` | GET POST DELETE | `autoindex off` and no index → 403 | `curl -i $H/noindex/` → 403 |
| `/multi-index/` | GET POST DELETE | Index fallback: `index index.html home.htm` serves `home.htm` | `curl $H/multi-index/` → "home.htm" page |
| `/gallery/` | GET | `alias` to `showcase/www/static/`, with listing | `curl -s -D- -o /dev/null $H/gallery/felis.jpg` → 200 `image/jpeg` |
| `/docs/` | GET | `root showcase/www/files`, so `/docs/x` → `files/docs/x` | `curl $H/docs/guide.txt` |
| `/readonly/` | GET | POST/DELETE → 405 | `curl -i -X DELETE $H/readonly/keep-me.txt` → 405 |
| `/private/` | GET POST DELETE | Location-level `error_page 404` overriding the server's | `curl $H/private/missing` → "404 Not Found (private)" |
| `/storage/` | GET POST DELETE | `upload_store` target (20m), listed with autoindex | `curl -i -F f=@/etc/hostname $H/storage/` → 201, then `curl -i -X DELETE $H/storage/hostname` → 200 |
| `/tiny-upload/` | POST | Same store, 1k limit → 413 for larger uploads | `head -c 3000 /dev/urandom > /tmp/b; curl -i -F f=@/tmp/b $H/tiny-upload/` → 413 |
| `/limits/` | GET POST | 100-byte body limit, POST to `/limits/echo.py` | `curl -i --data "$(printf 'x%.0s' {1..200})" $H/limits/echo.py` → 413 |
| `/profile/` | GET POST | Session profile page (bonus: cookies + sessions) | Browser: `/profile/`, save a name, reload: visit count goes up |
| `/profile_pics/` | GET | Images for the profile page | `curl -s -D- -o /dev/null $H/profile_pics/sushi.jpeg` → 200 |
| `/old/` | any | `return 301 /` | `curl -i $H/old/` → 301, `Location: /` |
| `/temp/` | any | `return 302 /files/` | `curl -i $H/temp/` → 302, `Location: /files/` |
| `/see-other/` | any | `return 303 /profile/` | `curl -i $H/see-other/` → 303 |
| `/keep-method/` | any | `return 307 /cgi/echo.py` (method + body kept) | `curl -iL --data hi $H/keep-method/` → 307, then `method: POST` |
| `/moved-for-good/` | any | `return 308 /cgi/echo.py` (method + body kept) | `curl -iL --data hi $H/moved-for-good/` → 308, then `method: POST` |
| `/nginx/` | any | `return 301 https://nginx.org/` (external) | Browser: `/nginx/` lands on nginx.org |
| `/health/` | any | `return 200 "OK - webserv is alive"` | `curl $H/health/` |
| `/gone/` | any | `return 404` → custom error page | `curl -i $H/gone/` → 404, custom page |

## Site B — `0.0.0.0:8082`

Root `showcase/www-b`, body limit 1k, **no** `error_page` (built-in error pages).

| Location | Methods | Purpose |
|---|---|---|
| `/` | GET | A different site on a different port |
| `/api/` | any | `return 200 "Hello from site B"` |

## Curl checks
### Configuration

| Check | Command | Expected |
|---|---|---|
| Custom 404 | `curl -i $H/missing` | 404, custom page |
| Location 404 | `curl -i $H/private/missing` | 404, "private" page |
| Default error page | `curl -i localhost:8082/nope` | 404, built-in page |
| Body under limit | `curl -i -X POST -H "Content-Type: plain/text" --data "BODY IS HERE" $H/limits/echo.py` | 200 |
| Body over limit | `curl -i -X POST -H "Content-Type: plain/text" --data "$(printf 'x%.0s' {1..200})" $H/limits/echo.py` | 413 |
| Upload over limit | `head -c 3000 /dev/urandom > /tmp/b; curl -i -F f=@/tmp/b $H/tiny-upload/` | 413 |
| `root` route | `curl $H/docs/guide.txt` | file from `www/files/docs/` |
| `alias` route | `curl -s -D- -o /dev/null $H/gallery/felis.jpg` | file from `www/static/` |
| Index fallback | `curl $H/multi-index/` | `home.htm` |
| DELETE allowed | `curl -F f=@/etc/hostname $H/storage/; curl -i -X DELETE $H/storage/hostname` | 201, 200 |
| DELETE not allowed | `curl -i -X DELETE $H/readonly/keep-me.txt` | 405 |

### Basic checks

| Check | Command | Expected |
|---|---|---|
| GET | `curl -i $H/` | 200 |
| Multi-file upload | `curl -i -F a=@/etc/hostname -F b=@/etc/os-release $H/storage/` | 201 |
| Get it back | `curl $H/storage/os-release` | file contents |
| DELETE missing | `curl -i -X DELETE $H/storage/nope` | 404 |
| Unknown method | `curl -i -X FOO $H/` | 501 |
| Wrong upload type | `curl -i -X POST -H "Content-Type: text/plain" --data hi $H/storage/` | 415 |
| No upload_store | `curl -i -X POST --data hi $H/` | 403 |
| HTTP/1.0 | `printf 'GET / HTTP/1.0\r\n\r\n' \| nc localhost 8080` | 200 |
| No Host | `printf 'GET / HTTP/1.1\r\n\r\n' \| nc localhost 8080` | 400 |
| Bad request line | `printf 'HELLO\r\n\r\n' \| nc localhost 8080` | 400 |
| Path traversal | `curl -i --path-as-is $H/../../etc/passwd` | 400 |
| Half-sent request | `(printf 'GET / HTTP/1.1\r\nHost: x\r\n'; sleep 8) \| nc localhost 8080` | 408 after ~5 s |
| Chunked body | `curl -H "Transfer-Encoding: chunked" --data "chunked hello" $H/cgi/echo.py` | `CONTENT_LENGTH: 13` |

### CGI

| Check | Command | Expected |
|---|---|---|
| GET + query | `curl "$H/cgi/env.py?name=webserv"` | 200, env listed |
| POST | `curl -X POST --data name=webserv $H/cgi/echo.py` | 200, body echoed |
| Working dir | `curl $H/cgi/cwd.py` | `cwd: …/showcase/www/cgi` |
| Status header | `curl -i "$H/cgi/status.py?code=409"` | 409 |
| Location header | `curl -i "$H/cgi/status.py?go=/files/"` | 302 |
| Python / Perl apps | `curl -X POST --data apple $H/cgi/py/cgi.py; curl -i -X DELETE "$H/cgi/py/cgi.py?name=apple"` (same for `pl/cgi.pl`) | 201, 204 |
| Syntax error / crash / bad output | `curl -i $H/cgi/syntax.py`, `error.py`, `badheaders.pl` | 502 |
| Infinite loop | `curl -i $H/cgi/loop.py` | 504 after 10 s |

### Ports

| Check | Command | Expected |
|---|---|---|
| Different sites | `curl localhost:8080/; curl localhost:8082/` | different pages |
| Same site, 2nd listen | `curl 127.0.0.1:8081/` | same as 8080 |
| Same port twice in a config | `printf 'server { listen 8090; root showcase/www; }\nserver { listen 8090; root showcase/www-b; }\n' > /tmp/dup.conf; ./webserv /tmp/dup.conf` | `Address already in use` |
| Two instances | run `./webserv showcase/full.conf` twice | 2nd exits with `Address already in use` |

## Siege

```bash
siege -b -c 25 -t 30S http://127.0.0.1:8080/
```

Availability should be above 99.5%, and `ps -o rss= -C webserv` should stay flat.

## Bonus

| Check | How | Expected |
|---|---|---|
| Sessions | `/profile/`: save a name and food, then reload | same `sid`, visit count goes up |
| Post/Redirect/Get | submit the profile form | 308 → 303 → 200 |
| Cookie from CGI | `curl -i -b visits=4 $H/cgi/cookie.py` | `Set-Cookie: visits=5` |

## Reset

```bash
find showcase/www/storage -type f ! -name .gitkeep -delete; rm -f showcase/www/cgi/*/data.json
```
