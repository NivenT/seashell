# seashell

This is a shell for linux and os x (mostly only tested on Ubuntu. Mileage may vary). It's early in development, so it's still missing a few basic features (e.g. background processes, a fleshed-out readme, etc.), but those and more are on their way.

## How to Build
Make sure you have `cmake` on your machine, and then do the following
```bash
git clone https://github.com/NivenT/seashell
cd seashell
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
home | This prints your home directory
alias | Creates shorthands for commands | `alias SHORTHAND COMMAND` | `alias cntlines "git ls-files \| grep -e c$ -e h$ \| xargs cat \| wc -l"` 
jobs | Lists all currently running jobs | `jobs`
% | Continues (first) stopped background job | `%`
kill | Sends signal to specifies processs | `kill --job ID --idx INDEX SIGNAL` <br> `kill --pid PID SIGNAL` | `kill --job 3 --idx 0 SIGCONT`

## Some Things That Might be Useful to Know

* You can tab complete builtin commands and filenames
  * Depending on how long its been since I've updated the readme, you may be able to tab complete even more
  * See [readline.c](https://github.com/NivenT/seashell/blob/master/src/readline.c) for the latest information
* You will get hints (yellow suggestions for finishing your input) when typing in the names of a builtin command
  * Again, there may also be more hints depending on how long it has been since I last updated the readme.
* Pipeing works (e.g. `cat file | grep key | wc -l`) but you can't yet write/read output/input from a file
  * That is, you can't do something like `echo write this to a file > output.txt`
