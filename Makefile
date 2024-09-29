build:
	gcc -o archiver cmd/archiver/main.c
	# gcc -o archiver $(find $(pwd)/ "*.c")
	# gcc -o archiver cmd/archiver/main.c internal/archiver/archiver.c

test-archive: build
	./archiver zip -i simple -o . 