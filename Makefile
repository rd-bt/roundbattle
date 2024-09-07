main: main.c battle-core.c battle.c battle.h battle-core.h moves.c term.c
	gcc -O2 -Wall main.c battle.c battle-core.c moves.c term.c -lm -o main -fsanitize=address
