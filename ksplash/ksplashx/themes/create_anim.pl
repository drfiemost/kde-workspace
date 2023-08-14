#!/usr/bin/perl

sub round($$)
{
  my ($value, $places) = @_;
  my $factor = 10**$places;
  return int($value * $factor + 0.5) / $factor;
}

my $inputFile=$ARGV[0];

if (not defined $inputFile) {
  die "Missing parameter input file\n";
}

my $command="convert $inputFile.png -blur 0x%.2f -alpha set -channel A -evaluate multiply %.2f +channel ${inputFile}_%02d.png";

for ($count = 0; $count < 30; $count += 1) {
    my $i=($count/29)*100;
    my $alpha=round($i/100, 2);
    my $blur=round((1 - $alpha)*10, 2);
    my $cmd=sprintf($command, $blur, $alpha, $count);
    #print($cmd."\n");
    system($cmd);
}

my $cmd="montage -mode concatenate -background none -tile 10x3 ${inputFile}_*.png ${inputFile}_anim.png";

#print($cmd."\n");
system($cmd);

system("rm ${inputFile}_??.png");
