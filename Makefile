CC := gcc
CFLAG := -Wall -Werror -O3 -fsanitize=address
all: rdbt nbtdump nbtedit list
rdbt: main.c battle.c battle.h battle-core.o term.c strmap.o locale.o species.c species.h info.c info.h utils.h utils.c menu.h menu.c player_data.h player_data.c nbt.o item.h item.c fun.c expr.o moves.o
	$(CC) $(CFLAG) main.c battle.c battle-core.o moves.o term.c strmap.o locale.o species.c info.c utils.c menu.c player_data.c nbt.o item.c expr.o -lm -lncursesw -o rdbt
nbtdump: nbtdump.c nbt.o
	$(CC) $(CFLAG) nbtdump.c nbt.o -o nbtdump
nbtedit: nbtedit.c nbt.o expr.h expr.o
	$(CC) $(CFLAG) nbtedit.c nbt.o expr.o -lm -lncurses -o nbtedit
list: list.c moves.o battle-core.o locale.o strmap.o species.c info.c species.h info.h
	$(CC) $(CFLAG) list.c moves.o battle-core.o locale.o strmap.o species.c info.c -lm expr.o -o list
expr.o: expr.c expr.h
	$(CC) $(CFLAG) expr.c -c -o expr.o
moves.o: moves.c moves.h
	$(CC) $(CFLAG) moves.c -c -o moves.o
strmap.o: strmap.c strmap.h
	$(CC) $(CFLAG) strmap.c -c -o strmap.o
nbt.o: nbt.c nbt.h
	$(CC) $(CFLAG) nbt.c -c -o nbt.o
battle-core.o: battle-core.c battle-core.h
	$(CC) $(CFLAG) battle-core.c -c -o battle-core.o
locale.o: locale.c locale.h
	$(CC) $(CFLAG) locale.c -c -o locale.o
.PHONY:
clean:
	rm -f rdbt nbtdump nbtedit list expr.o moves.o strmap.o nbt.o battle-core.o locale.o
