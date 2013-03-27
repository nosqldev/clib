#!/usr/bin/perl -w

use strict;
use Data::Dumper;

die "arg err: ./translate.pl source_code_filename" unless (@ARGV);

my $input_src_filename = $ARGV[0];
my $output_src_filename = $ARGV[0] . "_out.c";
my $output_precompile_file = $ARGV[0] . "_precompile.c";
my $output_final_src_file = $ARGV[0] . "_final.c";
my $source_code;
my $macro_code;
my @translate_header_files = ();
my @header_files = ();

open FH, ">$output_src_filename" or die "$!";

while (<>)
{
    if (/#include\s["<]/)
    {
        my ($filename) = /#include\s"(.*?)"/ if /#include\s"/;
        push(@header_files, $_) if (/#include\s</);
        push(@translate_header_files, $filename) if (defined($filename));
    }
    else
    {
        $source_code .= $_;
    }
}

foreach (@translate_header_files)
{
    open HEADER, $_ or die "$!";

    while (<HEADER>)
    {
        push(@header_files, $_), next if /#include/;
        $macro_code .= $_;
    }

    close(HEADER);
}

print FH $macro_code;
print FH $source_code;
close(FH);

system("cc -Wall -Wextra -E $output_src_filename > $output_precompile_file");

open FH, "$output_precompile_file" or die "$!";
open FSRC, ">$output_final_src_file" or die "$!";
foreach (@header_files)
{
    print FSRC $_;
}
while (<FH>)
{
    next if /^#\s\d+/;
    print FSRC $_;
}
close(FH);
close(FSRC);

system("astyle -s --indent-switches --indent-preprocessor --brackets=break --pad=oper --convert-tabs $output_final_src_file");
system("cc -Wall -Wextra -std=c99 -g $output_final_src_file -lm -lpthread");

unlink($output_src_filename);
unlink($output_precompile_file);
unlink($output_final_src_file . '.orig');
