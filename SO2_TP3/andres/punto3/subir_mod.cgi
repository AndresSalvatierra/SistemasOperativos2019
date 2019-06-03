#!/usr/bin/perl -wT

use strict;
use CGI;
use CGI::Carp qw ( fatalsToBrowser );
use File::Basename;

$CGI::POST_MAX = 1024 * 5000;
my $safe_filename_characters = "a-zA-Z0-9_.-";
my $upload_dir = "/var/www/html/andres/punto3";

my $query = new CGI;
my $filename = $query->param("subirmodulo");


if ( !$filename )
{
    print $query->header ( );
    print "There was a problem uploading your photo (try a smaller file).";
    exit;
}

my ( $name, $path, $extension ) = fileparse ( $filename,qr/\.[^.]*/ );
print $extension;
$filename = $name . $extension;
$filename =~ tr/ /_/;
$filename =~ s/[^$safe_filename_characters]//g;

if ( $filename =~ /^([$safe_filename_characters]+)$/ )
{
    $filename = $1;
}
else
{
    die "Filename contains invalid characters";
}

my $upload_filehandle = $query->upload("subirmodulo");

open ( UPLOADFILE, ">$upload_dir/$filename" ) or die "$!";
binmode UPLOADFILE;

while ( <$upload_filehandle> )
{
    print UPLOADFILE;
}

close UPLOADFILE;
#print $query->header ( );

#my $output_cmd = system("sudo insmod $upload_dir/$filename");
#if ($output_cmd ne 0) {
 #  error('ERROR! No se pudo instalar el modulo seleccionado.');
#}
#else{
 #  print $query->header ( );
  # printf "Se subió y se instaló con exito";
#}

#print <<END_HTML;

#<DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "DTD/xhtml1-strict.dtd">
#<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
    #<head>
       # <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
       # <title>Thanks!</title>
       # <style type="text/css">
      #      img {border: none;}
     #   </style>
    #</head>

   # <body>
  #      <p>Thanks for uploading your photo!</p>
 #   </body>
#</html>
#END_HTML
