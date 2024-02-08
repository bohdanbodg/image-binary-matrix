SCRIPTS_FOLDER = ./scripts

.PHONY: all
all:
	make build && make run

.PHONY: run
run:
	python3 $(SCRIPTS_FOLDER)/run.py

.PHONY: build
build:
	python3 $(SCRIPTS_FOLDER)/build.py