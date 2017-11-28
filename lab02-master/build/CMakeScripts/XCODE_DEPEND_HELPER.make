# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.Lab01.Debug:
/Users/bevintang/Downloads/School/CSC471/lab02-master/build/Debug/Lab01:
	/bin/rm -f /Users/bevintang/Downloads/School/CSC471/lab02-master/build/Debug/Lab01


PostBuild.Lab01.Release:
/Users/bevintang/Downloads/School/CSC471/lab02-master/build/Release/Lab01:
	/bin/rm -f /Users/bevintang/Downloads/School/CSC471/lab02-master/build/Release/Lab01


PostBuild.Lab01.MinSizeRel:
/Users/bevintang/Downloads/School/CSC471/lab02-master/build/MinSizeRel/Lab01:
	/bin/rm -f /Users/bevintang/Downloads/School/CSC471/lab02-master/build/MinSizeRel/Lab01


PostBuild.Lab01.RelWithDebInfo:
/Users/bevintang/Downloads/School/CSC471/lab02-master/build/RelWithDebInfo/Lab01:
	/bin/rm -f /Users/bevintang/Downloads/School/CSC471/lab02-master/build/RelWithDebInfo/Lab01




# For each target create a dummy ruleso the target does not have to exist
