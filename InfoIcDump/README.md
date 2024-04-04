# InfoIcDump
Open source utility for dumping `infoic.dll` and `infoic2Plus.dll` modules to an XML database file   
required by the opensource [minipro](https://gitlab.com/DavidGriffith/minipro) utility.



## Using of the InfoIcDump utility on Linux
#### Debian/Ubuntu
The provided `InfoicDump.exe` is a seflf-contained .net core application which can be run   
directly under Linux with the help of [Wine](https://wiki.winehq.org/Download) framework.   

You only need to install Wine without any other dependencies and create a clean 32 bit prefix.  
The 32 bit Wine prefix is needed because the `infoic.dll` and `infoic2Plus.dll` are also on 32 bit architecture.   
An existing 32 bit Wine prefix can be also used.  
```nohighlight 
WINEPREFIX=$HOME/wine_32 WINEARCH=win32 wine wineboot
git clone https://github.com/radiomanV/TL866.git
cd TL866/InfoicDump
```
Copy the `infoic.dll` or/and `infoic2Plus.dll` over the InfoicDump directory and run the utility:   
```nohighlight 
WINEPREFIX=$HOME/wine_32 WINEDEBUG=-all wine InfoicDump.exe
```
This will dump everything using default settings and provide a nice and clean output,    
removing the Wine debug gibbersish. 
If you have troubles remove the `WINEDEBUG=-all` part and run the utility again.   

The final dump will be found in `output` directory.   
You can customize this dump by providing some command line arguments:   
```
WINEPREFIX=$HOME/wine_32 WINEDEBUG=-all wine InfoicDump.exe --help
```

### Checkout source code and compile 
#### Install build dependencies
```nohighlight 
apt install dotnet-sdk-8.0
git clone https://github.com/radiomanV/TL866.git
cd TL866/InfoicDump
dotnet publish ./InfoicDump.csproj -c Release -r win-x86 /p:PublishSingleFile=true /p:PublishTrimmed=true /p:DebugType=embedded --output ./publish
cd publish
WINEPREFIX=$HOME/wine_32 WINEDEBUG=-all wine InfoicDump.exe
```
The `$HOME/wine_32` directory can be deleted if it is no longer needed.    

 
## Using of the InfoIcDump utility on Windows   

Using this utility under windows is straightforward because we don't need wine obviously.   
Assuming that you have [Git for windows](https://git-scm.com/download/win) installed use your favorite terminal app and:   
```nohighlight 
git clone https://github.com/radiomanV/TL866.git
cd TL866/InfoicDump
InfoicDump.exe
```

### Checkout source code and compile 
#### Install build dependencies
Install .NET 8.0 SDK from here: https://dotnet.microsoft.com/en-us/download/dotnet/8.0   
Then in your favorite terminal app:
```nohighlight 
git clone https://github.com/radiomanV/TL866.git
cd TL866/InfoicDump
dotnet publish InfoicDump.csproj -c Release -r win-x86 /p:PublishSingleFile=true /p:PublishTrimmed=true /p:DebugType=embedded --output publish
cd publish
InfoicDump.exe
```
   



