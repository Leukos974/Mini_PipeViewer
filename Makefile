##
## EPITECH PROJECT, 2025
## PipeViewer
## File description:
## Makefile
##

all:
	gcc -Wall -Wextra -Werror -o mini_pv mini_pv.c

clean:
	$(shell echo -n "" > output)

fclean:	clean
	rm -f mini_pv

re:	fclean all

test: clean
	@./mini_pv test > output

exemple: clean
	@pv test > output
