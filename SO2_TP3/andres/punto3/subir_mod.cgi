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
    print( "Hay un problema al subir el archivo");
    exit;
}

my ( $name, $path, $extension ) = fileparse ( $filename,qr/\.[^.]*/ );

if ($extension ne ".ko") {
    print("Usted ha ingresado un modulo con la extension incorrecta");
    exit;
} 

$filename = $name . $extension;
$filename =~ tr/ /_/;
$filename =~ s/[^$safe_filename_characters]//g;

if ( $filename =~ /^([$safe_filename_characters]+)$/ )
{
    $filename = $1;
}
else
{
    die "El archivo contiene caracteres invalidos";
}

my $upload_filehandle = $query->upload("subirmodulo");

open ( UPLOADFILE, ">$upload_dir/$filename" ) or die "$!";
binmode UPLOADFILE;

while ( <$upload_filehandle> )
{
    print UPLOADFILE;
}

close UPLOADFILE;

system ("sudo dmesg -C");
my $output_cmd = system("sudo insmod $upload_dir/$filename");
if ($output_cmd ne 0) {
  die('No se pudo instalar el modulo seleccionado.');
}
else{
  printf( system("dmesg"));
}
