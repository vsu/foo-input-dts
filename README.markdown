Foobar2000 DTS decoder plug-in
==============================

Description
-----------

This is a Foobar2000 plug-in that decodes DTS audio streams.
While there are many such plug-ins available, this one
uses the AC3Filter libraries (http://ac3filter.net), which
I have found to be sonically superior to the common libdca
based decoders and is more flexible in areas such as
supporting 24-bit PCM output.


Use and Configuration
---------------------

The output format and speaker configuration can be set in
the configuration panel.  For Foobar, this is generally
PCM 24 and 2 speaker.


Compilation
-----------

You will need:

Foobar2000 SDK (http://www.foobar2000.org/SDK)

Include files from the Windows Template Library (http://sourceforge.net/projects/wtl/)

AC3Filter tools library (http://ac3filter.net/wiki/Download_AC3Filter_tools)

  Current version is http://ac3filter.googlecode.com/files/ac3filter_tools_0_31b_src.zip
  Note: Exclude the directshow and directsound source files in the project when compiling

Project and solution files are currently for Visual Studio 2010.