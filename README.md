<p align="center">
  <img style="width: 200px;" src="https://github.com/littletijn/spooky-view/blob/main/docs/spooky-view-logo.svg" alt="Spooky View" />
</p>

# Spooky View
A Windows application to make any window transparent.

## System requirements
This application will run on Windows XP and newer.

## Downloads

The application is available from the Microsoft Store and Github.

### Microsoft Store
<a href="https://apps.microsoft.com/store/detail/spooky-view/9PB88ZKT0CDB">
  <img style="width: 300px;" src="https://github.com/littletijn/spooky-view/blob/main/docs/ms-store-badge-large-en.png" alt="Get it from Microsoft" />
</a>

### Github
https://github.com/littletijn/spooky-view/releases

When downloading from Github, make sure to choose the correct version.

- For 64-bits Windows, download the x64 version to make sure that x64 based apps can also be set transparent.
- For 32-bits Windows, download the x86 version. Use this version only on 32-bits computers. 
- For Windows 10 or 11 Arm-based PCs, download the ARM64 version for the best performance. 

## Known issues
- WPF (Windows Presentation Foundation) apps cannot be set transparent.
- When stopping Spooky View, some apps will not return opaque. Restart the affected apps to revert them.
- Only when Spooky View is run as administrator will it be able to make high privileges app transparent, like Windows Task Mananger.

## FAQ

*How do I enable automatic startup of Spooky View?*

It depends on the installed version. When using the MSI or portable version:
- Start Spooky View.
- Right click on the icon of the app in the Notification Area.
- Click on the "Settings..." option in the menu.
- Check the option "Start program when this user logs in".
- Click on "OK" button to save the changes and enable automatic startup.

When downloaded from the Microsoft Store or using the MSIX version:
- Open the Settings app of Windows.
- Click the "Apps" category.
- Click on the "Startup" option.
- Set the slider to enabled for the Spooky View item in the list of apps.

*How can I make high privileges apps (apps started as administrator) transparent?*

When the app is installed from the Microsoft Store, MSIX or MSI:
- Find the app in the start menu.
- Right click on the app.
- Click on the "Run as administrator" option.

When the portable app is downloaded and extracted:
- Find the executable of the app (SpookyView_version_Portable.exe).
- Right click on the icon of the executable.
- Click on the "Run as administrator" option.

## Building
This project can be build with Visual Studio 2022. Install the "Desktop Development with C++" workload and the "C++ Windows XP Support for VS 2017 (v141) tools" component. Open the Solution file "SpookyView.sln" and build the SpookyView project within.

## License
Spooky View is licensed under the GNU GLP v3.0 or later. See the LICENSE file for more details.
