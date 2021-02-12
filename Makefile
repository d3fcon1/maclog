.PHONY: all
all: libmaclog tests

.PHONY: libmaclog
libmaclog:
	mkdir -p ./lib
	$(MAKE) -C ./src -f libmaclog.mk

.PHONY: tests
tests: libmaclog
	mkdir -p ./bin
	$(MAKE) -C ./tests -f tests.mk	

.PHONY: clean
clean:
	$(MAKE) -C ./src -f libmaclog.mk clean
	$(MAKE) -C ./tests -f tests.mk clean
	rm -rf ./bin
	rm -rf ./lib

