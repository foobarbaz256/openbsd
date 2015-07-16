# The client writes a message to Sys::Syslog native method.
# The syslogd writes it into a file and through a pipe.
# The syslogd does not pass it via IPv4 UDP to the IPv6 loghost.
# Find the message in client, file, pipe, syslogd log.
# Check that the syslogd logs the error.

use strict;
use warnings;

our %args = (
    syslogd => {
	loghost => '@udp4://[::1]',
	loggrep => {
	    qr/syslogd: bad hostname "\@udp4:\/\/\[::1\]"/ => 2,
	    get_testgrep() => 1,
	},
    },
    server => {
	noserver => 1,
    },
);

1;
