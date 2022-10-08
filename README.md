# LTSPath

Forwarder of arguments for applications that don't support UTF8 characters in paths.
The long paths are converted to short path.

It might not work if the path is on a network drive and you don't have the access to the parents of the path.

## How to use

 - Compile with VS Studio 2022
- Make a file ltspath.txt where LTSPath is located containing the path to the application you want to forward the arguments (example: "c:\Apps\myapp.exe" without the quotes)
- Replace the calls to your application with this one
- Profit

Logs are stored in ltspath.log where LTSPath is located.


## License

LTSPath is available on Github under the MIT license.