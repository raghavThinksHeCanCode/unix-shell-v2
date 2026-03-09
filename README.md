# Unix shell
A custom Unix shell written entirely from scratch in C. Supports job control, pipes, signal handling and sequential/conditional command execution.

## Overview
This is a commandline shell that allows you to execute commands in terminal. The main highlight of this shell is that it supports job control, i.e, multiple processes can execute in the background while the shell itself is executing some other program in foreground.

This is an educational project I created to understand the process model and programming interface of Unix and unix-like operating systems. It also gave me a better understanding of the terminal subsystem.

All of the code is written entirely in C. All of the modules are written from scratch without use of any external libraries.

## Installation


## Features
1. Multiple process execution using `&&`, `||` and `;`
2. Interprocess communication using pipes `|`
3. Background execution using `&`
4. Suspension of running process by pressing `Ctrl-z`
5. Termination of running process by pressing `Ctrl-c`
6. Asynchronous waiting for jobs; suspension, termination or exiting of jobs executing in background is reported by the shell
7. Builtins
   1. `cd <dir>`    - Change directory (expansion not supported)
   2. `exit`        - Exit the shell
   3. `exec <prog>` - Launch an external program (arguments to program not supported)
   4. `jobs`        - List all jobs running in background
   5. `fg <job-no>` - Put job with number `<job-no>` into foreground and continue it. Simply use `fg n` instead of `fg %n`
   6. `bg <job-no>` - Continue job with number `<job-no>` in the background. Syntax like `fg`

## Architecture and working
The shell roughly works like this:
1. Take user input
2. Tokenize the input into identifiable tokens
3. Parse the tokens into some intermediate representation
4. Execute the parsed representation
5. Repeat

