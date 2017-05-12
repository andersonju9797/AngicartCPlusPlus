# AngicartCPlusPlus
Angicart C++ Code

More install guidelines and commands for C++ code
See below for my quick version of how to install and run the software.
-------------
You first need Xcode
>xcode-select --install
You might need wxmac
>brew install wxmac
Install homebrew
> /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
Check for libraries
> /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"


Using code

Should now be able to open David’s software within Xcode, and can look at code (in source folder), and can import example image files and output data.
Executable code is named: minSurfTests.xcodeproj
There is no gui yet for choosing file names or even example files such as
small_easy_test of FITC…. folders.
Instead, you must open minSurfTests.cpp in XCode.
First, make sure the path is right for the folders you want to analyze. Path line is specified near end of file and reads:
string pathToImages("/Users/vsavage/Desktop/minimalSurfaces_share");
Then look for the line near the end of the same file that is:
string imageDir(pathToImages + "/FITC-MCA0_N12_NIH001_s1_4"); int imStart(0), imEnd(66); double voxdims[] = {1.648, 1.648, 0.492};  thresh = 0.22; string lengthUnit("um");
The part “/FITC-MCA0_N12_NIH001_s1_4” is the piece you can replace with the folder name for whatever images you want to analyse such as “small_easy_test”, and the voxel dimensions must be specified as the arguments to voxdims[] = {1.648, 1.648, 0.492}, which more generally is voxdims[] = {x, y, z} where x, y, and z are the 3D dimensions for an image as measured in units of microns.
After this is all completed, you can now run the code and see the movie as it runs and get the output file for vessel measurements by either clicking the play button at the top left of the XCode window or by going to the “Product” menu and selecting “Run” or by simultaneously pressing “Command” plus “R”.
