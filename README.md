# ecet4640-lab6

Instructions for this assignment are located [at this link](https://students.pennwest.edu/chen/ecet4640/lab/phase3security.html)

## Some todos
- ~~log when client disconnects, log user if extant~~
- implement advertisement
- ~~implement registered users files~~
- ~~implement lockfile~~
- ~~implement command line args~~
    ~~- 'headless' command again, to ./nohup itself~~
    ~~- stop command~~
    ~~- reset command~~
- ~~implement interrupt signals, sigterm signals, for graceful shutdown (write lockfile, ensure text is normal color)~~
- ~~logging to logfile?~~

- Note: the server IP is: 158.83.254.100   but it doesn't appear we can expose ports currently. Should talk to Prof. chen about this. 

# Protocol

Send to Client:

~Message~ A text message
~Error~ A server-side error, accompanying a connection close?

Send to Server:
userID as a string... 
REGISTER - register your user
HELP - get a list of available commands
EXIT - disconnect from the server
MYINFO - get info about yourself

ADVERTISEMENT - send ascii art //define size of ascii art

Their buffer size should be at least 11k bytes to accomodate Advertisements

Bonus Possibilities:

1. ADVERTISEMENT - send them ascii art of the software development club
2. let them change GPA / age / etc
 SET GPA 2.5
 SET AGE 99
 RANDOM GPA
 RANDOM AGE
3. Messages
 SEND MESSAGE <userid> <msg>
 READ MESSAGES
4. WHO (who is online?) - Unregistered?????


# Some high-priority Todos
