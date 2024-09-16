main: main.c battle-core.c battle.c battle.h battle-core.h moves.c moves.h term.c strmap.c strmap.h
	gcc -O2 -Wall main.c battle.c battle-core.c moves.c term.c strmap.c -lm -o main -fsanitize=address
