#!/usr/bin/perl

while (1) {
#Event: time 1682750683.674406, type 1 (EV_KEY), code 46 (KEY_C), value 1
    my $l = <>;
    if ($l =~ /EV_KEY.* code \d* \(KEY_(.*)\), value (\d*)/) {
        if ($2 == "1") {
            print "+";
        } elsif ($2 == "0") {
            print "-";
        } else {
            next;
        }
        print "$1\n";
	select()->flush(); # flush STDOUT
    }
}

