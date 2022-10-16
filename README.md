# LTSPath

Forwarder of arguments for applications that don't support unicode characters in paths.

The paths in the arguments and the current directory containing unicode characters are converted to a new path pointing to a symbolic link.

If this didn't work (need admin rights), the long paths are then converted to short path. The short path conversion might not work if the short path was not created by the OS or if the path is on a network drive and you don't have the access to the parents of the path.

## How to use

- Download the release or compile with VS Studio 2022
- Make a folder "ltspath" where "LTSPath.exe" is located.
- Inside this folder, create a file "path.txt" containing the path to the application you want to forward the arguments (example: "c:\Apps\myapp.exe" without the quotes)
- Replace the calls to your application with this one
- (Optional but recommended) To enable the creation of symbolic links, the program needs admin rights (thanks Microsoft...). To do so, right click on LTSPath.exe and go to "Properties", then in the "Compatibility" tab, check "run this program as an administrator" and apply.
- Profit

Logs are stored in ltspath\logs.log where LTSPath is located.

## License

LTSPath is available on Github under the MIT license.