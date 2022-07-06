G_PARAMS = -Wall -g

all: fstool

src = fstool.c ata.c bitmap.c simplefs.c

fstool: $(src)
	gcc $(G_PARAMS) -o $@ $(src)

.PHONY = clean

clean:
	rm -rf fstool