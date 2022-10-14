# SSHELL: A shell inside another shell

## Summary
This program 'sshell' will act as a functional shell where it runs commands inputted by the user on the command line. 
The program 'sshell', is suppose to imitate the actions of a regular shell, like
the Linux command line. It takes in commands from the user and executes them,

It also allows for the execute onf three built in commands not provided by the 
hell.  as we;ll   

## Implementation
The implementation of this program follows three distinct steps:
1. Parsing the command line.
Our implementation uses a custom struct called 'CommandLine' which holds values
for an array of arguments, a file name, and a snippet of the full command line. 

First, we allocated the proper amount of memory to store four instances of our
CommandLine struct. These structs were contained in an array of pointers
called 'cl' for 'command line'. This was in preparation for the ability to pipe
up to four times. 

We then split the full command line into snippets, using '|' as the delimiter.
In each successive snippet, we increased a 'numCommands' count to keep track of
the number of separate commands to be executed by a pipe. The snippets were
stored in the 'cmd' variable in each command's respective CommandLine instance.

From here, we call a parse function that parses the command line snippet of
each used instance in cl. The function iterates through each character in the
instance's cmd string, saving the proper characters into the instance's argument
array. When the function finds a space, it ignores all adjacent spaces until it
finds the beginning of a new word. It then increases an argument count variable
and starts adding to the next index in the argument array. When the fuction
finds a greater than or less than sign, it saves it's index and uses it to
extract the input or output's file name. Finally, it fills in the remaining
argument spaces with NULL.

The shell then checks if the given command is a built in command. If it is, it
executes. If it isn't the shell utitlizes the fork+exec+wait method to carry out
all other commands. Included with this is forking a first time, checking if the
command needs to be piped, then forking additional times given the situation.
Since our piping method was never solved, we weren't able to integrate the
system in a single for loop and instead used an if statement to separate the
situations requiring piping and the situations where piping is not involved. 

Following the single command method, our program forks with the parent waiting
for the status of the child and the child executing the bulk of the command.
First the child checks if input/output redirection is required and modifies the
file descriptors accordingly. Next, it calls execvp and saves the return value
as an int called 'evpReturn'. This variable is used to determine the return
status when the child process exits.
2. Executing fork()+exec()+wait() to ensure the sshell runs commands while the sshell itself is running.
3. Testing

For testing, we mostly used the examples in the prompt, as well as our own
additions to manual test the functionality of our code. When we stumbled across
issues, we used gdb to deduce the source of our errors. We also used the
autograder on Gradescope to advise us about the official shortcomings of our
program.

## Sources

The only code copied directly from any source was code provided to us in
lectures and lecture slides. We did, however, use internet sources to advise our
code in different places. Most of the content we found to be useful came from
Geeksforgeeks.org and tutorialspoint.com, both of which helped inform us on
functions such as strtok and others.
