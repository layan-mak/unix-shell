# unix-shell
Implementation of a **command-line interpreter** that provides a command-line user interface for Unix-like OS. The shell implemented is called: Smash 🔥

The program waits for commands that will be typed by the user and executes them (back and forth).

Smash works as follows:
- The program can execute a small number of **built-in commands**, you can see the commands in commands.h.
- Support of **foreground and background** (using the ampersand symbol &) commands and **stopping** them (by pressing Ctrl+Z).
- Jobs are stored in the jobs list (implemented by a linked list): Any job that is either (1) sent to the background or (2) stopped  will be added to the jobs list, allowing users to query and manage them later by their associated job ID.
For example: You can ask the shell to print all the jobs controlled by it via the jobs command.
- Support of **Pipes and IO redirection**.
- **Signal handling**: The Smash supports Ctrl+C (SIGINT), Ctrl+Z (SIGSTP) and SIG_ALRM (SIGKILL).
- The program can execute external commands.

Assumptions: 
- Only up to 100 processes can run simultaneously.

How to use: 
- Download the files, compile and run them on a Unix VM, and use the shell prompt to execute commands.


