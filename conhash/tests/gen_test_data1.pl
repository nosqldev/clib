#!/usr/bin/perl -w

my $dict_file = "/usr/share/dict/web2";
my $output_line;
open FH, $dict_file;
for ($i=0; $i<1000; $i++)
{
    $output_line = 'add_string("';
    chomp($output_line .= <FH>);
    $output_line .= '", "';
    chomp($output_line .= <FH>);
    $output_line .= '");';
    print $output_line, "\n";
}
close(FH);
