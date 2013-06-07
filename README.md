FileTransfer
===========

-----------------------------------------------------------------------------------------------
CLIENT:
compile command: gcc -I/usr/local/project/fileTrans/server -o main main.c command.c input.c

FIRST:
connect 127.0.0.1 (this command shold run first,127.0.0.1 should change be the server ip,the port is 8000 )

THEN:
get /usr/local/a.txt /usr/ ( put local file /usr/local/a.txt to server folder /usr/, the name is a.txt)
put /usr/local/a.txt /usr/ ( get server file /usr/local/a.txt to local folder /usr/, the name is a.txt)
bye  (disconnect)
--------------------------------------------------------------------------------------------------
SERVER:


