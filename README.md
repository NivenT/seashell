# seashell

This is a shell for linux and os x (mostly only tested on Ubuntu. Mileage may vary). 

<p align="center"><img src="https://github.com/NivenT/seashell/blob/master/demo.gif" width="500" /></p>

It has plenty of the features one would expect, such as

* Pipeing (e.g. `cat file | grep key | wc -l`)
* Input/output files (e.g. `cat < original.txt > copy.txt`)
* Background processes (e.g. `sleep 100 &`)
* The ability to stop (by pressing `CTRL+Z`) and continue (`%`) processes 
* Comments (e.g. `ls # this is a comment`)
* Compound commands (e.g. `make && ./seashell`)

See the bottom of this README for more information.

## How to Build
Make sure you have `cmake` on your machine, and then do the following
```bash
git clone https://github.com/NivenT/seashell
cd seashell
git submodule update --init --recursive
mkdir build
cd build
cmake ../
make
```

And then simply run with `./seashell`.

## Builtin Commands

These are listed at the top of [builtins.c](https://github.com/NivenT/seashell/blob/master/src/builtins.c), but may be implemented elsewhere (e.g. `bookmark.c`). Currently, there is

Name | Description | Usage | Example
---- | ----------- | ----- | -------
exit/quit | These exit the program
cd | This changes the directory | cd DIRECTORY | `cd path/to/fldr`
bookmark | This manages a list of directories that you can easily swap between | `bookmark --save DIRECTORY --name NAME` Saves a bookmark to the list<br> `bookmark --goto NAME` Changes directory. `NAME` can be the one given to `--name` or the number printed at the beginning of a line by `--list`<br> `bookmark --list` Lists all the bookmarks | `bookmark --save path/to/fld --name screenplays`
home | This prints your home directory | `home`
alias | Creates shorthands for commands | `alias SHORTHAND COMMAND` | `alias cntlines "git ls-files \| grep -e c$ -e h$ \| xargs cat \| wc -l"` 
jobs | Lists all currently running jobs | `jobs`
% | Continues (first) stopped background job | `%`
kill | Sends signal to specifies processs | `kill --job ID --idx INDEX SIGNAL` <br> `kill --pid PID SIGNAL` | `kill --job 3 --idx 0 SIGCONT`
history | Prints out all commands stored in the shell's history | `history`
fg/bg | Move a job to the foreground or background | `fg JOBID` <br> `bg JOBID`

## Some Things That Might be Useful to Know

* There's decent tab completion.
  * You can tab complete filenames as well as any programs stored in PATH
  * builtin commands can also be tab completed
  * Certain common inputs (e.g. `sudo apt-get install`) can also be tab completed, one word at a time
  * You can tab complete packages when typing `sudo apt install` and the like
    * At startup, the shell fetches a list of all packages avaiable to apt
  * when cloning git repositories, you can tab complete the name of a repository
    * When you type something like `git clone https://github.com/<user>`, the shell fetches a list of repos belonging to that user
  * See [readline.c](https://github.com/NivenT/seashell/blob/master/src/readline.c) for the latest information
* You will ocassionally get hints (yellow suggestions for finishing your input). See [readline.c](https://github.com/NivenT/seashell/blob/master/src/readline.c) for complete information.
  * Builtin commands receive hints
  * Certain common commands also receive hints
  * Your 10 most frequently used commands also receive hints
    * In the demo, this wasn't limited to 10 so that I would get extra help remembering what I wanted to type
  * There may also be more hints depending on how long it has been since I last updated the readme.
* At startup, the program reads in a `.seashellrc` file from your home path and executes every command in it
  * See [rcfile.c](https://github.com/NivenT/seashell/blob/master/src/rcfile.c) for more details
* At the top of [main.c](https://github.com/NivenT/seashell/blob/master/src/main.c), there is a TODO list of the features I'm planning on adding next.
* The seashell in the icon was stolen from [here](https://pixabay.com/vectors/seashell-shell-ocean-beach-sea-1531572/)
