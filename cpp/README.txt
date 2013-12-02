bbLog README file

10-24-2013 Sterling Peet <sterling.peet@gatech.edu>

The BeagleBone Black has a lot of software in the onboard flash, and updates
need to be done via writing a new image to a uSD card.  Flash the uSD contents
to the BBB by using the instructions at Adafruit's website.

  To use the UARTs, they must be enabled via the uEnv.txt file on the BBB. Here
is how to do that:

http://blog.pignology.net/2013/05/getting-uart2-devttyo1-working-on.html

uEnt.txt:
optargs=quiet drm.debug=7 capemgr.enable_partno=BB-UART2

Installed packages:
opkg install cmake
opkg install boost
opkg install boost-dev

I just do a "scp -r bbLog root@beaglebone.local:." to get the project onto
the BBB.  Then I log into the BBB and run cmake, then make, then make install.

----

  I accidentally managed to brown-out the board while the ethernet was plugged
in.  I was trying to install some new software packages into the default image,
and rebooted the board.  The result wiped all the data from the internal flash,
including the uboot interface.  Fortunately, writing a new uSD image to the uSD
resulted in the BBB automatically trying to re-flash itself with the uSD image.

Explanation of how the software update process actually works:
http://www.crashcourse.ca/wiki/index.php/BBB_software_update_process

Repository of all BeagleBoard/BBB Images:
http://dominion.thruhere.net/koen/angstrom/beaglebone/