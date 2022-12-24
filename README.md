This was created during my Operating Systems 1 class which required us to build a shell.
  The learning outcomes of this assignment were:
    * Describe the Unix process API
    * Write programs using the Unix process API
    * Explain the concept of signals and their uses
    * Write programs using the Unix API for signal handling 
    * Explain I/O redirection and write programs that can employ I/O redirection

A Make file is included to compile the code.
The executible can be run within a bash shell: "./smallsh"

Here is an example run using smallsh. Note that CTRL-C has no effect towards the bottom of the example, when it's used while sitting at the command prompt:
    $ smallsh
    : /bin/timedatectl
                   Local time: Sun 2021-10-17 15:26:30 PDT
               Universal time: Sun 2021-10-17 22:26:30 UTC
                     RTC time: Sun 2021-10-17 22:26:31
                    Time zone: America/Los_Angeles (PDT, -0700)
    System clock synchronized: yes
                  NTP service: active
              RTC in local TZ: no
    : ls > junk
    : status
    exit value 0
    : cat junk
    junk
    smallsh
    smallsh.c
    : wc < junk > junk2
    : wc < junk
           3       3      23
    : test -f badfile
    : status
    exit value 1
    : wc < badfile
    cannot open badfile for input
    : status
    exit value 1
    : badfile
    badfile: no such file or directory
    : sleep 5
    ^Cterminated by signal 2
    : status &
    terminated by signal 2
    : sleep 15 &
    background pid is 4923
    : ps
      PID TTY          TIME CMD
     4923 pts/0    00:00:00 sleep
     4564 pts/0    00:00:03 bash
     4867 pts/0    00:01:32 smallsh
     4927 pts/0    00:00:00 ps
    :
    : # that was a blank command line, this is a comment line
    :
    background pid 4923 is done: exit value 0
    : # the background sleep finally finished
    : sleep 30 &
    background pid is 4941
    : kill -15 4941
    background pid 4941 is done: terminated by signal 15
    : pwd
    /nfs/stak/users/chaudhrn/CS344/prog3
    : cd
    : pwd
    /nfs/stak/users/chaudhrn
    : cd CS344
    : pwd
    /nfs/stak/users/chaudhrn/CS344
    : echo 4867
    4867
    : echo $$
    4867
    : ^C^Z
    Entering foreground-only mode (& is now ignored)
    : date
     Mon Jan  2 11:24:33 PST 2017
    : sleep 5 &
    : date
     Mon Jan  2 11:24:38 PST 2017
    : ^Z
    Exiting foreground-only mode
    : date
     Mon Jan  2 11:24:39 PST 2017
    : sleep 5 &
    background pid is 4963
    : date
     Mon Jan 2 11:24:39 PST 2017
    : exit
    $
