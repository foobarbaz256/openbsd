# The client writes a message to Sys::Syslog native method.
# The syslogd writes it into a file and through a pipe.
# The syslogd passes it via UDP to the loghost.
# The server receives the message on its UDP socket.
# Find the message in client, file, pipe, syslogd, server log.
# Create a ktrace dump of the client and check that sendsyslog(2)
# has been used.

use strict;
use warnings;

our %args = (
    client => {
	ktrace => {
	    qr/CALL  sendsyslog/ => 2,
	    qr/GIO   fd -1 wrote \d+ bytes/ => 2,
	    qr/RET   sendsyslog 0/ => 2,
	},
    },
);

1;
