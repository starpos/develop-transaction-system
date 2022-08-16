.PHONY: build clean rebuild

build: book.pdf version_str.txt
	cp -a book.pdf develop-transaction-system-v$$(cat version_str.txt).pdf

version_str.txt: VERSION
	cat VERSION |sed -e 's/\./_/g' > $@

book.pdf: *.re *.yml
	rake pdf

clean:
	rm -f book.pdf

rebuild:
	$(MAKE) clean
	$(MAKE) build
