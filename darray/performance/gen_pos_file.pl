#!/usr/bin/perl -w

# © Copyright 2010 jingmi. All Rights Reserved.
#
# +-----------------------------------------------------------------------+
# | gen pos array for random_access_darray.c                              |
# +-----------------------------------------------------------------------+
# | Author: jingmi@gmail.com                                              |
# +-----------------------------------------------------------------------+
# | Created: 2010-04-13 19:04                                             |
# +-----------------------------------------------------------------------+

use strict;
use Data::Dumper;

&main($ARGV[0]);

sub main
{
    my $num = shift || 10_000;
    open FH, ">pos";
    for (1..$num)
    {
        print FH int(rand($num)) . "\n";
    }
}

__END__
# vim: set expandtab tabstop=4 shiftwidth=4 foldmethod=marker:
