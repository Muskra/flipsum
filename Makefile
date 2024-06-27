default:
	clang -g flipsum.c -o flipsum

test1:
	clang -g flipsum.c -o flipsum && ./flipsum test1.txt && shasum -a 256 test1.txt test1.txt.flip

test2:
	clang -g flipsum.c -o flipsum && ./flipsum test2.txt && shasum -a 256 test2.txt test2.txt.flip
