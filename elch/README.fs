Hi,

This is a new README for the fs-version of my chess fonts.  The
METAFONT sources for them can be found in fselch.
  'fs' stand's for "feenschach", a wellknown fairy chess magazine. I
have done this version of my chess font, to support the printing of the
book "Umwandlungen in Maerchen figuren". This book will be published
around the end of 1993. The editor -- bernd ellinghoven -- urged me to
use shapes of the figures which resemble very closley those used in
printing the feenschach magazine. I have promised to do so and the
result of this promise can be found in the subdirectory fselch.
  This version of the elch fonts is also an enancement of the older
elch fonts. These lacked in some respect good digitizing on low
resolutions.  This is much better with fselch. The encoding scheme
is the same, and both versions of the font can be used with the macros
designed for this encoding scheme. 
  Also improved is the file structure: there is only a small file for
every pointsize, which sets up the mode and pointsize; all other things
are done in chbase.mf and the files for the six shapes.
  This also shows the greatest lack of the font: it is simply scaled
for the different pointsizes; it is not specially designed. Especially
the small point sizes at low resolution deserve a better handling
of its small size and the small amount of pixels available. This was
not so much a problem for printing the book, since i produced the
printing originals at 600dpi with magstep 1 on DinA4 size which will be
shrinked by optical means to DinA5 in the final print.
  For this book I develloped a very special format based on PlainTeX.
This format is by no means suitable as a generic macro package to
use this fonts. But Stefan Hoening is preparing a style file for LaTeX
which used this fonts. His work will be based on a first LaTeX style
which was done by Thomas Brand. If and when this will be given to
public use cannot be telled now, but you will find it here at this
place.

					Elmar Bartel
					11.Nov.1993

This version of the elch fonts is also given to the public under
the GNU Public Licence. A copy is contained in the file LICENCE.
