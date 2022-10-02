#Generate executable
sshell: sshell.c
	gcc -Wall -Wextra -Werror -o sshell sshell.c

#Clean generated files
clean:
	rm -f sshell
