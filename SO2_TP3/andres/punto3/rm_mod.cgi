#!/usr/bin/perl -wT
 
use strict;
use warnings;
use CGI;
use CGI::Carp qw/fatalsToBrowser/;
use File::Basename;
 
$CGI::POST_MAX = 1024 * 5000; #adjust as needed (1024 * 5000 = 5MB)
$CGI::DISABLE_UPLOADS = 0; #1 disables uploads, 0 enables uploads
 
my $query = CGI->new;
 
unless ($CGI::VERSION >= 2.47) { 
   error('Your version of CGI.pm is too old. You must have verison 2.47 or higher to use this script.')
}
 
my $upload_dir = '/var/www/html/andres/punto3';
 
# a list of valid characters that can be in filenames
my $filename_characters = 'a-zA-Z0-9_.-';
 
my $file = $query->param("rm_modulo") or error('No file selected for upload.') ;

# get the filename and the file extension
# this could be used to filter out unwanted filetypes
# see the File::Basename documentation for details
my ($filename,undef,$ext) = fileparse($file,qr{\..*});

if ($ext ne ".ko") {
    error("Usted no ha ingresado un modulo incorrecto.");
} 

# append extension to filename
#$filename .= $ext;
 
# convert spaces to underscores "_"
$filename =~ tr/ /_/;
 
# remove illegal characters
$filename =~ s/[^$filename_characters]//g;
 
# satisfy taint checking
if ($filename =~ /^([$filename_characters]+)$/) {
   $filename = $1;
}
else{
   error("El archivo es invalido, puede contener caracteres invalidos")
}
 
my $output_cmd = system("sudo rmmod $filename");

if ($output_cmd ne 0) {
   error('ERROR! No se pudo encontrar el modulo');
}
else{
   print"Removido"
}
 
sub error {
   print $query->header(),
         $query->start_html(-title=>'Error'),
         shift,
         $query->end_html;
   exit(0);
}
