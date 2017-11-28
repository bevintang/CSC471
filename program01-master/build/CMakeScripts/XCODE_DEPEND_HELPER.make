# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.raster.Debug:
/Users/bevintang/Downloads/School/CSC471/program01-master/build/Debug/raster:
	/bin/rm -f /Users/bevintang/Downloads/School/CSC471/program01-master/build/Debug/raster


PostBuild.raster.Release:
/Users/bevintang/Downloads/School/CSC471/program01-master/build/Release/raster:
	/bin/rm -f /Users/bevintang/Downloads/School/CSC471/program01-master/build/Release/raster


PostBuild.raster.MinSizeRel:
/Users/bevintang/Downloads/School/CSC471/program01-master/build/MinSizeRel/raster:
	/bin/rm -f /Users/bevintang/Downloads/School/CSC471/program01-master/build/MinSizeRel/raster


PostBuild.raster.RelWithDebInfo:
/Users/bevintang/Downloads/School/CSC471/program01-master/build/RelWithDebInfo/raster:
	/bin/rm -f /Users/bevintang/Downloads/School/CSC471/program01-master/build/RelWithDebInfo/raster




# For each target create a dummy ruleso the target does not have to exist
