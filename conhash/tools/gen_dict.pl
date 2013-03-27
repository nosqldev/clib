#!/usr/bin/perl -w

use strict;

die "arg error: source_dict new_dict words_num" if (@ARGV != 3);

my @words = ();
my $dict_cnt = `wc -l $ARGV[0]`;
chomp($dict_cnt);
$dict_cnt =~ s/^\s*(\d+).*$/$1/g;
$dict_cnt = int($dict_cnt);
my $loop_cnt = int(int($ARGV[2]) / $dict_cnt + 1);
#print $dict_cnt, "--\t", $loop_cnt, "==\n";

open DICT, $ARGV[0] or die "$!";

while (<DICT>)
{
    chomp;
    next if (length($_) >= 20);
    my $word = $_;
    push @words, $word;
    for (1..$loop_cnt)
    {
        push @words, $word . $_;
    }
}

close(DICT);

open NEWDICT, ">$ARGV[1]", or die "$!";

for (1..$ARGV[2])
{
    print NEWDICT shift(@words), "\n";
}

close(NEWDICT);
