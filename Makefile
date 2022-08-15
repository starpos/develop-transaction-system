.PHONY: build clean rebuild

build: book.pdf

book.pdf: *.re *.yml
	rake pdf

clean:
	rm -f book.pdf

rebuild:
	$(MAKE) clean
	$(MAKE) build
