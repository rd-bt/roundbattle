CC := gcc
CFLAG := -Wall -O3 -fsanitize=address
all: rdbt nbtdump
rdbt: main.c battle-core.c battle.c battle.h battle-core.h moves.c moves.h term.c strmap.c strmap.h locale.c locale.h species.c species.h info.c info.h utils.h utils.c menu.h menu.c player_data.h player_data.c nbt.h nbt.c
	$(CC) $(CFLAG) main.c battle.c battle-core.c moves.c term.c strmap.c locale.c species.c info.c utils.c menu.c player_data.c nbt.c -lm -lncurses -o rdbt
nbtdump: nbtdump.c nbt.h nbt.c
	$(CC) $(CFLAG) nbtdump.c nbt.c -lm -o nbtdump
.PHONY:
clean:
	rm -f rdbt nbtdump
