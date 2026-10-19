PYTHON ?= python3
VERSION ?= 1.0.0
.DEFAULT_GOAL := help

.PHONY: help check serve package clean

help:
	@echo "make check    - run offline repository checks (links, docs parity, data, secrets)"
	@echo "make serve    - serve web/ at http://localhost:8080"
	@echo "make package  - build the deterministic source archive + SHA-256"
	@echo "make clean    - remove dist/ and transient syntax-check files"

check:
	$(PYTHON) scripts/check_repo.py

serve:
	$(PYTHON) scripts/serve_web.py

package:
	$(PYTHON) scripts/package_release.py --version $(VERSION) --output dist/SJRC-AutoReader-Repo.zip

clean:
	rm -rf dist .web-check.js
