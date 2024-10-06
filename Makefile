main: main.c battle-core.c battle.c battle.h battle-core.h moves.c moves.h term.c strmap.c strmap.h locale.c locale.h species.c species.h info.c info.h utils.h utils.c menu.h menu.c player_data.h player_data.c
	gcc -O2 -Wall main.c battle.c battle-core.c moves.c term.c strmap.c locale.c species.c info.c utils.c menu.c player_data.c -lm -lncurses -o main -fsanitize=address
