main: main.c battle-core.c battle.c battle.h battle-core.h moves.c
	gcc -O2 -Wall main.c battle.c battle-core.c moves.c -lm -o main
