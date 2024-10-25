CC := gcc
CFLAG := -Wall -O3 -fsanitize=address
all: rdbt nbtdump nbtedit
rdbt: main.c battle-core.c battle.c battle.h battle-core.h moves.c moves.h term.c strmap.c strmap.h locale.c locale.h species.c species.h info.c info.h utils.h utils.c menu.h menu.c player_data.h player_data.c nbt.h nbt.c item.h item.c
	$(CC) $(CFLAG) main.c battle.c battle-core.c moves.c term.c strmap.c locale.c species.c info.c utils.c menu.c player_data.c nbt.c item.c -lm -lncursesw -o rdbt
nbtdump: nbtdump.c nbt.h nbt.c
	$(CC) $(CFLAG) nbtdump.c nbt.c -o nbtdump
nbtedit: nbtedit.c nbt.h nbt.c expr.h expr.c
	$(CC) $(CFLAG) nbtedit.c nbt.c expr.c -lm -lncurses -o nbtedit
.PHONY:
clean:
	rm -f rdbt nbtdump nbtedit
