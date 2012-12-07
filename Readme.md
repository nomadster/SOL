This is my final term project, written for the Computer Science course _"Laboratorio di Sistemi Operativi"_  (SOL) at the University of Pisa.

It is a simple client-server car sharing system.

The server has multiple _worker_ threads, each of which is responsible of handling a communication 
session with a single client.
These session basically consist of request to update and/or retrieve information on a mutual-exclusively
accessed graph, representing a network of roads, plus some authetication and error handling.
The communication is done via UNIX Sockets, using a pre-defined protocol.
Once the _worker_ has validated the request, this is queued in a specific structure to be served later by a dedicated
thread that is called _match_.

This thread is in charge of satisfying client request, dispatching replys to the right recipient.

There is also a bash script that makes some statistics among the matches between the requests. You can find it in `src/carstat`.

This project makes use of POSIX-Threads.

If you can read italian you may want to take a look at [this](https://www.dropbox.com/s/wtnibuugnq5dgem/SOL.pdf) report.