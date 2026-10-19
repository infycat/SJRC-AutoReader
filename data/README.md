# Data files

Machine-readable companions to the documentation. Endpoint tables are curated
from the public 2.9.0 client reference; the full decompiled tree is not included.
No live API call is needed to use these files.

## Files

| File | Rows / shape | Source |
|---|---|---|
| `api-endpoints.csv` | 98 rows, columns `http,path,method,class,description` | Curated from the public 2.9.0 client reference |
| `api-endpoints.json` | 98 methods, 67 unique `METHOD + path` pairs | Same, normalised by module |
| `api-endpoints-by-module.md` | Grouped summary | Generated from the JSON |
| `source-stats.json` | SDK levels, upstream reference counts, included-source counts, artifact sizes and SHA-256 | Research notes and release binaries |

## Conventions

- `path` keeps literal Retrofit placeholders such as `{id}`. It is not expanded
  with sample values.
- Methods whose URL is supplied at runtime use the synthetic path
  `(dynamic @Url)`. Keep this row; removing it changes the count.
- `class` is the owning smali class name, retained as a research index; source
  files are not included here.
- `description` is `?` when the method name does not explain the operation; do
  not invent a description.
- Endpoint presence does **not** mean this project calls it. The shipped clients
  use only the routes listed in `docs/en/api.md`.
- `source-stats.json` is a snapshot. If an artifact is rebuilt, update its size
  and SHA-256 together with `SHA256SUMS`.

## Reproducing

The original analysis is documented in
[`docs/en/reverse-engineering.md`](../docs/en/reverse-engineering.md#recovering-the-endpoint-list).
The full extraction input is not distributed. For a separately obtained
reference copy, endpoint strings can be searched with:

```bash
grep -rn "reading-time" sjrc-source/smali | head
python3 - <<'PY'
# sanity-check the JSON counts
import json
d = json.load(open('data/api-endpoints.json'))
print(d['method_count'], d['unique_endpoint_count'], len(d['methods']))
PY
```

Expected: `98 67 98`.

The old source-side `hdr` field was a corrupted androguard representation and is
deliberately excluded from these tables.

## Privacy

No tokens, account identifiers, email addresses, school names, or book content
appear here. Endpoint paths and class names are protocol metadata.
