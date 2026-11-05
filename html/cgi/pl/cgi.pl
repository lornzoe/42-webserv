#!/usr/bin/perl
use strict;
use warnings;
use JSON::PP;
use File::Basename qw(dirname);
use Cwd qw(abs_path);

my $data_file = dirname(abs_path($0)) . "/data.json";
my $json = JSON::PP->new->utf8;

sub save_data {
    my ($data) = @_;
    open(my $fh, '>', $data_file) or die "cannot write $data_file: $!";
    print $fh $json->encode($data);
    close $fh;
}

sub load_data {
    save_data([]) unless -e $data_file;
    open(my $fh, '<', $data_file) or die "cannot read $data_file: $!";
    local $/;
    my $content = <$fh>;
    close $fh;
    return $json->decode($content);
}

sub respond {
    my ($status, $reason, $body) = @_;
    print "Status: $status $reason\r\n";
    print "Content-Type: application/json\r\n";
    if (defined $body) {
        my $payload = $json->encode($body);
        print "Content-Length: " . length($payload) . "\r\n\r\n";
        print $payload;
    } else {
        print "\r\n";
    }
}

sub url_decode {
    my ($s) = @_;
    $s =~ tr/+/ /;
    $s =~ s/%([0-9A-Fa-f]{2})/chr(hex($1))/eg;
    return $s;
}

my $method = $ENV{REQUEST_METHOD} // "";
my $data   = load_data();

if ($method eq "GET") {
    respond(200, "OK", $data);
}
elsif ($method eq "POST") {
    my $len = $ENV{CONTENT_LENGTH} || 0;
    my $name = "";
    read(STDIN, $name, $len) if $len > 0;

    if ($name eq "") {
        respond(400, "Bad Request", { error => "Name cannot be empty" });
    } elsif (grep { $_ eq $name } @$data) {
        respond(409, "Conflict", { error => "Duplicate name" });
    } else {
        push @$data, $name;
        save_data($data);
        respond(201, "Created", $data);
    }
}
elsif ($method eq "DELETE") {
    my $name = "";
    for my $pair (split /&/, $ENV{QUERY_STRING} // "") {
        my ($k, $v) = split /=/, $pair, 2;
        $name = url_decode($v // "") if defined $k && $k eq "name";
    }

    if (!grep { $_ eq $name } @$data) {
        respond(404, "Not Found", { error => "Name not found" });
    } else {
        save_data([ grep { $_ ne $name } @$data ]);
        respond(204, "No Content");
    }
}
else {
    print "Status: 405 Method Not Allowed\r\nAllow: GET, POST, DELETE\r\n\r\n";
}