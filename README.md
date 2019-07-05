# seashell

This is a shell for linux and os x (mostly only tested on Ubuntu. Mileage may vary). It's early in development, so it's still missing a few basic features (e.g. background processes, good tab completion, a fleshed-out readme), but those and more are on their way.

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

Name | Description | Usage
---- | ----------- | -----
exit/quit | These exit the program
cd | This changes the directory | cd DIRECTORY
bookmark | This manages a list of directories that you can easily swap between | `bookmark --save DIRECTORY --name NAME` Saves a bookmark to the list<br> `bookmark --goto NAME` Changes directory. `NAME` can be the one given to `--name` or the number printed at the beginning of a line by `--list`<br> `bookmark --list` Lists all the bookmarks
home | This prints your home directory

## Some Things That Might be Useful to Know

* You can tab complete builtin commands and names of files in the current directory
  * Depending on how long its been since I've updated the readme, you may be able to tab complete even more
* Pipeing works (e.g. `cat file | grep key | wc -l`) but you can't yet write/read output/input from a file
  * That is, you can't do something like `echo write this to a file > output.txt`
