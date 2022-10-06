# PretzelWatcher
A watcher which restarts pretzel.rocks desktop app in a specified interval.

## Motivation

The current version of pretzel.rocks music player (2.11.4) gets slower the longer it plays music which makes it hard to use after a while (clicking a button takes several seconds etc.) It also consumes a lot of RAM (several GB) and music stutters after a while.

To fix these issues in a workaround manner, this tool restarts pretzel desktop app after a specified interval. Before the app is restarted, it waits until the current played song is finished so the restart isn't noticed by the audience.

## How to use

Pretzel Desktop app must run before `PretzlWatcher` is started. Pretzl also needs to be configured to write the current played track into a text file, which can be enabled in the right hand side menu (`File Output` > `Write Track Info to file`). This file path needs to be provided to `PretzlWatcher` as it is used to find out when the current song has finished playing.

```
Usage: PretzelWatcher.exe <Track info file> <Restart interval minutes>
Example: PretzelWatcher.exe C:\\Users\\Public\\Documents\\current_song.txt 120
```
