all: xkeepbloom test

xkeepbloom:
	make -f Makefile.xkeepbloom

test:
	make -f Makefile.test

