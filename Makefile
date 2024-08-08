main: main.c battle-core.c battle.c battle.h battle-core.h
	gcc -O2 -Wall main.c battle.c battle-core.c -o main
